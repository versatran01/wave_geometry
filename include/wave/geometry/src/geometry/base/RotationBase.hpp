/**
 * @file
 * @author lkoppel
 */

#ifndef WAVE_GEOMETRY_ROTATIONBASE_HPP
#define WAVE_GEOMETRY_ROTATIONBASE_HPP

namespace wave {

/** Base class for rotations in SO(3) */
template <typename Derived>
class RotationBase : public TransformBase<Derived> {
    using Scalar = internal::scalar_t<Derived>;
    using OutputType = internal::plain_output_t<Derived>;

 public:
    template <typename T>
    using BaseTmpl = RotationBase<T>;

    using RotationType = Derived;
    using TranslationType = typename internal::add_frames<
      LeftFrameOf<Derived>,
      LeftFrameOf<Derived>,
      RightFrameOf<Derived>>::template to<Zero<Translation<Eigen::Matrix<Scalar, 3, 1>>>>;

    // Return self as rotation(), to fit Transform interface
    auto rotation() const & -> const Derived & {
        return this->derived();
    }

    auto rotation() & -> Derived & {
        return this->derived();
    }

    auto rotation() && -> Derived && {
        return this->derived();
    }

    auto translation() const -> TranslationType {
        return TranslationType{};
    }
};


/** Rotates a point. Also known as a coordinate map.
 *
 * @f[ SO(3) \times R^3 \to R^3 @f]
 */
template <typename L, typename R>
auto operator*(const RotationBase<L> &lhs, const TranslationBase<R> &rhs)
  -> Rotate<L, R> {
    return Rotate<L, R>{lhs.derived(), rhs.derived()};
}

WAVE_OVERLOAD_FUNCTION_FOR_RVALUES(operator*, Rotate, RotationBase, TranslationBase)

namespace internal {

/** Base for traits of a rotation leaf expression using an Eigen type for storage.*/
template <typename Derived>
struct rotation_leaf_traits_base;

template <template <typename...> class Tmpl, typename ImplType_>
struct rotation_leaf_traits_base<Tmpl<ImplType_>> : leaf_traits_base<Tmpl<ImplType_>>,
                                                    frameable_transform_traits {
    template <typename NewImplType>
    using rebind = Tmpl<NewImplType>;

    using ImplType = ImplType_;
    using Scalar = typename ImplType::Scalar;
    using TangentType = RelativeRotation<Eigen::Matrix<Scalar, 3, 1>>;
    static constexpr int TangentSize = 3;

    // Each leaf must define its own PlainType. There is no consistent place to get it
    // because Eigen matrices have PlainObject in the class, Quaternion in traits, and
    // AngleAxis nowhere
};

/** Implementation of Random for a rotation leaf
 *
 * Produces a random rotation on SO(3), constructing it from a quaternion
 */
template <typename Leaf, typename Rhs>
auto evalImpl(expr<Random, Leaf>, const RotationBase<Rhs> &) {
    return Leaf{randomQuaternion<internal::scalar_t<Leaf>>()};
}

/** Implementation of Identity for a rotation leaf */
template <typename Leaf, TICK_REQUIRES(internal::is_derived_rotation<Leaf>{})>
auto evalImpl(expr<Convert, Leaf>, const Identity<Leaf> &) {
    return Leaf{traits<Leaf>::ImplType::Identity()};
}

}  // namespace internal
}  // namespace wave

#endif  // WAVE_GEOMETRY_ROTATIONBASE_HPP
