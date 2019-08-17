#include <octoon/offline_mesh_renderer_component.h>
#include <octoon/offline_feature.h>
#include <octoon/game_scene.h>
#include <octoon/mesh_filter_component.h>
#include <octoon/transform_component.h>
#include <octoon/video/render_system.h>
#include <octoon/game_prefabs.h>

#include <RadeonProRender.h>

namespace octoon
{
	OctoonImplementSubClass(OfflineMeshRendererComponent, OfflineRenderComponent, "OfflineMeshRenderer")

	OfflineMeshRendererComponent::OfflineMeshRendererComponent() noexcept
		: rprShape_(nullptr)
		, rprMaterial_(nullptr)
	{
	}

	OfflineMeshRendererComponent::OfflineMeshRendererComponent(model::MaterialPtr&& material) noexcept
		: rprShape_(nullptr)
		, rprMaterial_(nullptr)
	{
		this->setMaterial(std::move(material));
	}

	OfflineMeshRendererComponent::OfflineMeshRendererComponent(const model::MaterialPtr& material) noexcept
		: rprShape_(nullptr)
		, rprMaterial_(nullptr)
	{
		this->setMaterial(material);
	}

	OfflineMeshRendererComponent::~OfflineMeshRendererComponent() noexcept
	{
	}

	void
	OfflineMeshRendererComponent::uploadMatData(const model::Material& mat) noexcept
	{
		auto feature = this->getGameScene()->getFeature<OfflineFeature>();
		if (!feature)
			return;

		if (!this->rprMaterial_)
		{
			std::string name;
			std::string path;
			std::string textureName;

			math::float3 base = math::float3(1.0f, 0.0f, 1.0f);
			math::float3 ambient;

			mat.get(MATKEY_NAME, name);
			mat.get(MATKEY_PATH, path);
			mat.get(MATKEY_TEXTURE_DIFFUSE, textureName);
			mat.get(MATKEY_COLOR_DIFFUSE, base);
			mat.get(MATKEY_COLOR_AMBIENT, ambient);

			rprMaterialSystemCreateNode(feature->getMaterialSystem(), RPR_MATERIAL_NODE_UBERV2, &rprMaterial_);
			rprMaterialNodeSetInputF(rprMaterial_, "uberv2.diffuse.color", base[0], base[1], base[2], 1.0f);

			auto texture = GamePrefabs::instance()->createTexture(path + textureName);
			if (texture)
			{
				try
				{
					std::uint8_t* data;
					auto desc = texture->getTextureDesc();
					texture->map(0, 0, desc.getWidth(), desc.getHeight(), 0, (void**)& data);

					bool hasAlpha = false;
					std::uint8_t channel = 3;
					switch (desc.getTexFormat())
					{
						case hal::GraphicsFormat::B8G8R8A8UNorm:
							hasAlpha = true;
							channel = 4;
							break;
						case hal::GraphicsFormat::R8G8B8A8UNorm:
							hasAlpha = true;
							channel = 4;
							break;
						case hal::GraphicsFormat::B8G8R8A8SRGB:
							hasAlpha = true;
							channel = 4;
							break;
						case hal::GraphicsFormat::R8G8B8A8SRGB:
							hasAlpha = true;
							channel = 4;
							break;
					}

					rpr_image_format imageFormat = { channel, RPR_COMPONENT_TYPE_UINT8 };
					rpr_image_desc imageDesc;
					imageDesc.image_width = desc.getWidth();
					imageDesc.image_height = desc.getHeight();
					imageDesc.image_depth = 1;
					imageDesc.image_row_pitch = desc.getWidth() * imageFormat.num_components;
					imageDesc.image_slice_pitch = imageDesc.image_row_pitch * desc.getHeight();

					std::vector<std::uint8_t> array(desc.getWidth() * desc.getHeight() * imageFormat.num_components);
					for (std::size_t y = 0; y < desc.getHeight(); y++)
					{
						for (std::size_t x = 0; x < desc.getWidth(); x++)
						{
							auto dst = ((desc.getHeight() - 1 - y) * desc.getWidth() + x) * imageFormat.num_components;
							auto src = (y * desc.getWidth() + x) * imageFormat.num_components;

							for (std::uint8_t i = 0; i < imageFormat.num_components; i++)
								array[dst + i] = data[src + i];
						}
					}

					rpr_image image_;
					rprContextCreateImage(feature->getContext(), imageFormat, &imageDesc, array.data(), &image_);

					rpr_material_node textureNode;
					rprMaterialSystemCreateNode(feature->getMaterialSystem(), RPR_MATERIAL_NODE_IMAGE_TEXTURE, &textureNode);
					rprMaterialNodeSetInputImageData(textureNode, "data", image_);

					std::uint32_t layers = RPR_UBER_MATERIAL_LAYER_DIFFUSE | RPR_UBER_MATERIAL_LAYER_REFLECTION;
					/*if (hasAlpha)
						layers |= RPR_UBER_MATERIAL_TRANSPARENCY;*/

					rprMaterialNodeSetInputU(rprMaterial_, "uberv2.layers", layers);

					if (layers & RPR_UBER_MATERIAL_LAYER_DIFFUSE)
						rprMaterialNodeSetInputN(rprMaterial_, "uberv2.diffuse.color", textureNode);
							
					if (layers & RPR_UBER_MATERIAL_TRANSPARENCY)
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.transparency", 0.5f, 1.0f, 1.0f, 1.0f);

					if (layers & RPR_UBER_MATERIAL_LAYER_REFLECTION)
					{
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.reflection.ior", 1.5f, 1.5f, 1.5f, 1.5f);
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.reflection.color", 1.0f, 1.0f, 1.0f, 1.0f);
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.reflection.metalness", 0.0f, 0.0f, 0.0f, 0.0f);
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.reflection.roughness", 0.9f, 0.9f, 0.9f, 0.9f);
					}

					if (layers & RPR_UBER_MATERIAL_LAYER_REFRACTION)
					{
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.refraction.ior", 1.5f, 1.5f, 1.5f, 1.5f);
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.refraction.roughness", 1.0f, 1.0f, 1.0f, 1.0f);
						rprMaterialNodeSetInputF(rprMaterial_, "uberv2.refraction.color", 1.0f, 1.0f, 1.0f, 1.0f);
					}
							
					texture->unmap();
				}
				catch (...)
				{
					texture->unmap();
				}
			}
		}
	}

	void
	OfflineMeshRendererComponent::uploadMeshData(const model::Mesh& mesh) noexcept
	{
		auto feature = this->getGameScene()->getFeature<OfflineFeature>();
		if (feature)
		{
			this->uploadMatData(*this->getMaterial());

			if (this->rprShape_)
			{
				rprSceneDetachShape(feature->getScene(), this->rprShape_);
				rprObjectDelete(this->rprShape_);
				this->rprShape_ = nullptr;
			}

			math::uint1s faceArray(mesh.getIndicesArray().size() / 3, 3);

			rprContextCreateMesh(feature->getContext(),
				mesh.getVertexArray().data()->ptr(), mesh.getVertexArray().size() / 3, sizeof(math::float3),
				mesh.getNormalArray().data()->ptr(), mesh.getNormalArray().size() / 3, sizeof(math::float3),
				mesh.getTexcoordArray().data()->ptr(), mesh.getTexcoordArray().size() / 2, sizeof(math::float2),
				(rpr_int*)mesh.getIndicesArray().data(), sizeof(rpr_int),
				(rpr_int*)mesh.getIndicesArray().data(), sizeof(rpr_int),
				(rpr_int*)mesh.getIndicesArray().data(), sizeof(rpr_int),
				(rpr_int*)faceArray.data(), faceArray.size(),
				&this->rprShape_);

			rprShapeSetShadow(this->rprShape_, true);
			rprShapeSetShadowCatcher(this->rprShape_, true);
			rprShapeSetVisibility(this->rprShape_, true);
			rprShapeSetLayerMask(this->rprShape_, this->getGameObject()->getLayer());
			rprShapeSetObjectGroupID(this->rprShape_, (rpr_uint)this->getGameObject()->id());
			rprShapeSetMaterial(this->rprShape_, this->rprMaterial_);
			rprShapeSetTransform(this->rprShape_, false, this->getComponent<TransformComponent>()->getTransform().ptr());

			rprSceneAttachShape(feature->getScene(), this->rprShape_);
		}
	}

	GameComponentPtr
	OfflineMeshRendererComponent::clone() const noexcept
	{
		auto instance = std::make_shared<OfflineMeshRendererComponent>();
		instance->setName(this->getName());
		instance->setMaterial(this->getMaterial() ? (this->isSharedMaterial() ? this->getMaterial() : this->getMaterial()->clone()) : nullptr, this->isSharedMaterial());

		return instance;
	}

	void
	OfflineMeshRendererComponent::onActivate() noexcept
	{
		this->addComponentDispatch(GameDispatchType::MoveAfter);
		this->addMessageListener("octoon:mesh:update", std::bind(&OfflineMeshRendererComponent::onMeshReplace, this, std::placeholders::_1));
		this->sendMessage("octoon:mesh:get", nullptr);

		auto feature = this->getGameScene()->getFeature<OfflineFeature>();
		if (feature)
			feature->addOfflineListener(this);
	}

	void
	OfflineMeshRendererComponent::onDeactivate() noexcept
	{
		this->removeComponentDispatch(GameDispatchType::MoveAfter);
		this->removeMessageListener("octoon:mesh:update", std::bind(&OfflineMeshRendererComponent::onMeshReplace, this, std::placeholders::_1));

		auto feature = this->getGameScene()->getFeature<OfflineFeature>();
		if (feature && this->rprShape_)
		{
			feature->removeOfflineListener(this);
			rprSceneDetachShape(feature->getScene(), this->rprShape_);
		}

		if (this->rprShape_)
		{
			rprObjectDelete(this->rprShape_);
			this->rprShape_ = nullptr;
		}

		if (this->rprMaterial_)
		{
			rprObjectDelete(this->rprMaterial_);
			this->rprMaterial_ = nullptr;
		}
	}

	void
	OfflineMeshRendererComponent::onMoveBefore() noexcept
	{
	}

	void
	OfflineMeshRendererComponent::onMoveAfter() noexcept
	{
		if (this->rprShape_)
		{
			auto transform = this->getComponent<TransformComponent>();
			rprShapeSetTransform(this->rprShape_, false, transform->getTransform().ptr());

			auto feature = this->getGameScene()->getFeature<OfflineFeature>();
			if (feature)
				feature->setFramebufferDirty(true);
		}
	}

	void
	OfflineMeshRendererComponent::onMeshReplace(const runtime::any& mesh_) noexcept
	{
		auto mesh = runtime::any_cast<model::MeshPtr>(mesh_);
		if (mesh && this->getMaterial())
			this->uploadMeshData(*mesh);
	}

	void
	OfflineMeshRendererComponent::onMaterialReplace(const model::MaterialPtr& material) noexcept
	{
	}

	void
	OfflineMeshRendererComponent::onLayerChangeAfter() noexcept
	{
		if (this->rprShape_)
			rprShapeSetLayerMask(this->rprShape_, this->getGameObject()->getLayer());
	}
}