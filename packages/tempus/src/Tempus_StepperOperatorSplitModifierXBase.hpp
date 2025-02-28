// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#ifndef Tempus_StepperOperatorSplitModifierXBase_hpp
#define Tempus_StepperOperatorSplitModifierXBase_hpp

#include "Tempus_config.hpp"
#include "Tempus_SolutionHistory.hpp"
#include "Tempus_StepperOperatorSplitAppAction.hpp"

namespace Tempus {

/** \brief Base ModifierX for StepperOperatorSplit.
 *
 *  This class provides a means to modify just the solution values
 *  (i.e., \f$x\f$ and \f$dot{x}\f$), and nothing else, but time and
 *  timestep are also provided.
 *
 *  Users deriving from this class can access and change the solution
 *  during the timestep (e.g., limiting the solution for monoticity).
 *  It is expected that the user knows what changes are allowable without
 *  affecting the Stepper correctness, performance, accuracy and stability
 *  (i.e., USER BEWARE!!).
 *
 *  The locations of the StepperOperatorSplitModifierXBase::MODIFIER_TYPE
 *  which correspond to the AppAction calls
 *  (StepperOperatorSplitAppAction::ACTION_LOCATION) are shown in the
 *  algorithm documentation of the StepperOperatorSplit.
 */

template <class Scalar>
class StepperOperatorSplitModifierXBase
  : virtual public Tempus::StepperOperatorSplitAppAction<Scalar> {
 private:
  /* \brief Adaptor execute function
   *
   *  This is an adaptor function to bridge between the AppAction
   *  interface and the ModifierX interface.  It is meant to be private
   *  and non-virtual as deriving from this class should only need to
   *  implement the modify function.
   *
   *  For the ModifierX interface, this adaptor maps the
   *  StepperOperatorSplitAppAction::ACTION_LOCATION to the
   *  StepperOperatorSplitModifierX::MODIFIERX_TYPE, and only pass the solution
   *  (\f$x\f$ and/or \f$\dot{x}\f$ and other parameters to the modify
   *  function.
   */
  void execute(
      Teuchos::RCP<SolutionHistory<Scalar> > sh,
      Teuchos::RCP<StepperOperatorSplit<Scalar> > stepper,
      const typename StepperOperatorSplitAppAction<Scalar>::ACTION_LOCATION
          actLoc)
  {
    using Teuchos::RCP;

    MODIFIER_TYPE modType                    = X_BEGIN_STEP;
    RCP<SolutionState<Scalar> > workingState = sh->getWorkingState();
    const Scalar time                        = workingState->getTime();
    const Scalar dt                          = workingState->getTimeStep();
    RCP<Thyra::VectorBase<Scalar> > x;

    switch (actLoc) {
      case StepperOperatorSplitAppAction<Scalar>::BEGIN_STEP: {
        modType = X_BEGIN_STEP;
        x       = workingState->getX();
        break;
      }
      case StepperOperatorSplitAppAction<Scalar>::BEFORE_STEPPER: {
        modType = X_BEFORE_STEPPER;
        x       = workingState->getX();
        break;
      }
      case StepperOperatorSplitAppAction<Scalar>::AFTER_STEPPER: {
        modType = X_AFTER_STEPPER;
        x       = workingState->getX();
        break;
      }
      case StepperOperatorSplitAppAction<Scalar>::END_STEP: {
        modType = XDOT_END_STEP;
        if (workingState->getXDot() != Teuchos::null)
          x = workingState->getXDot();
        else
          x = stepper->getStepperXDot();
        break;
      }
      default:
        TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error,
                                   "Error - unknown action location.\n");
    }

    this->modify(x, time, dt, modType);
  }

 public:
  /// Indicates the location of application action (see algorithm).
  enum MODIFIER_TYPE {
    X_BEGIN_STEP,      ///< Modify \f$x\f$ at the beginning of the step.
    X_BEFORE_STEPPER,  ///< Modify \f$x\f$ before the implicit solve.
    X_AFTER_STEPPER,
    XDOT_END_STEP  ///< Modify \f$\dot{x}\f$ at the end of the step.
  };

  /// Modify solution based on the MODIFIER_TYPE.
  virtual void modify(Teuchos::RCP<Thyra::VectorBase<Scalar> > /* x */,
                      const Scalar /* time */, const Scalar /* dt */,
                      const MODIFIER_TYPE modType) = 0;
};

}  // namespace Tempus

#endif  // Tempus_StepperOperatorSplitModifierXBase_hpp
