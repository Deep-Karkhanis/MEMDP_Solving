#include <AIToolbox/MDP/Model.hpp>

namespace AIToolbox {
    namespace MDP {
        Model Model::makeFromTrustedData(size_t s, size_t a, TransitionTable && t, RewardTable && r, double d) {
            return Model(s, a, std::move(t), std::move(r), d);
        }

        Model::Model(size_t s, size_t a, TransitionTable && t, RewardTable && r, double d) :
            S(s), A(a), discount_(d), transitions_(t), rewards_(r), rand_(Impl::Seeder::getSeed()) {}

        Model::Model(size_t s, size_t a, double discount) : S(s), A(a), discount_(discount), transitions_(A, Matrix2D(S, S)), rewards_(A, Matrix2D(S, S)),
                                                       rand_(Impl::Seeder::getSeed())
        {
            // Make transition table true probability
            for ( size_t a = 0; a < A; ++a ) {
                transitions_[a].setIdentity();
                rewards_[a].fill(0.0);
            }
        }

        void Model::setTransitionFunction(const Matrix3D & t) {
            // First we verify data, without modifying anything...
            for ( size_t a = 0; a < A; ++a ) {
                for ( size_t s = 0; s < S; ++s ) {
                    if ( t[a].row(s).minCoeff() < 0.0 ||
                         !checkEqualSmall(1.0, t[a].row(s).sum()) ) throw std::invalid_argument("Input transition table does not contain valid probabilities.");
                }
            }

            // Then we copy.
            transitions_ = t;
        }

        void Model::setRewardFunction( const Matrix3D & r ) {
            rewards_ = r;
        }

        std::tuple<size_t, double> Model::sampleSR(size_t s, size_t a) const {
            size_t s1 = sampleProbability(S, transitions_[a].row(s), rand_);

            return std::make_tuple(s1, rewards_[a](s, s1));
        }

        double Model::getTransitionProbability(size_t s, size_t a, size_t s1) const {
            return transitions_[a](s, s1);
        }

        double Model::getExpectedReward(size_t s, size_t a, size_t s1) const {
            return rewards_[a](s, s1);
        }

        void Model::setDiscount(double d) {
            if ( d <= 0.0 || d > 1.0 ) throw std::invalid_argument("Discount parameter must be in (0,1]");
            discount_ = d;
        }

        bool Model::isTerminal(size_t s) const {
            bool answer = true;
            for ( size_t a = 0; a < A; ++a ) {
                if ( !checkEqualSmall(1.0, transitions_[a](s, s)) ) {
                    answer = false;
                    break;
                }
            }
            return answer;
        }

        size_t Model::getS() const { return S; }
        size_t Model::getA() const { return A; }
        double Model::getDiscount() const { return discount_; }

        const Model::TransitionTable & Model::getTransitionFunction() const { return transitions_; }
        const Model::RewardTable &     Model::getRewardFunction()     const { return rewards_; }

        const Matrix2D & Model::getTransitionFunction(size_t a) const { return transitions_[a]; }
        const Matrix2D & Model::getRewardFunction(size_t a)     const { return rewards_[a]; }
    }
}
