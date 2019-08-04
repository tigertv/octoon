#include <octoon/box_collider_component.h>

#include <octoon/rigidbody_component.h>
#include <octoon/physics_feature.h>
#include <octoon/game_app.h>
#include <octoon/game_scene.h>
#include <octoon/runtime/except.h>

namespace octoon
{
    OctoonImplementSubClass(BoxColliderComponent, ColliderComponent, "BoxCollider")

	BoxColliderComponent::BoxColliderComponent() noexcept
		: width_(1.0)
		, height_(1.0)
		, depth_(1.0)
    {
    }

	BoxColliderComponent::BoxColliderComponent(const math::float3& size) noexcept
		: width_(size.x)
		, height_(size.y)
		, depth_(size.z)
	{
	}

	BoxColliderComponent::BoxColliderComponent(float x, float y, float z) noexcept
		: width_(x)
		, height_(y)
		, depth_(z)
	{
	}

    BoxColliderComponent::~BoxColliderComponent()
    {
    }

	void
	BoxColliderComponent::setWidth(float width) noexcept
	{
		if (shape_)
			shape_->setWidth(width);
		this->width_ = width;
	}

	void 
	BoxColliderComponent::setHeight(float height) noexcept
	{
		if (shape_)
			shape_->setHeight(height);
		this->height_ = height;
	}

	void
	BoxColliderComponent::setDepth(float depth) noexcept
	{
		if (shape_)
			shape_->setDepth(depth);
		this->depth_ = depth;
	}

	float
	BoxColliderComponent::getWidth() const noexcept
	{
		return width_;
	}

	float
	BoxColliderComponent::getHeight() const noexcept
	{
		return height_;
	}

	float
	BoxColliderComponent::getDepth() const noexcept
	{
		return depth_;
	}

    GameComponentPtr
	BoxColliderComponent::clone() const noexcept
    {
		auto instance = std::make_shared<BoxColliderComponent>();
		instance->setName(this->getName());
		instance->setWidth(this->getWidth());
		instance->setHeight(this->getHeight());
		instance->setDepth(this->getDepth());
		return instance;
    }

	std::shared_ptr<physics::PhysicsShape>
	BoxColliderComponent::getShape() noexcept
	{
		return this->shape_;
	}

    void
	BoxColliderComponent::onActivate() except
    {
		auto physicsFeature = this->getGameScene()->getFeature<PhysicsFeature>();
		if (physicsFeature)
		{
			physics::PhysicsBoxShapeDesc boxDesc;
			boxDesc.width = width_;
			boxDesc.height = height_;
			boxDesc.depth = depth_;
			shape_ = physicsFeature->getContext()->createBoxShape(boxDesc);
		}
    }

    void
	BoxColliderComponent::onDeactivate() noexcept
    {
		shape_.reset();
		shape_ = nullptr;
    }
}