#ifndef __AML_PSDK_SACAM_H
#define __AML_PSDK_SACAM_H

#include <aml-psdk/gta_base/Vector.h>
#include "../enum/Camera.h"

struct CEntity;
struct CVehicle;
struct CPed;

DECL_CLASS(CCam)
    // Member values
    bool          m_bBelowMinDist;
    bool          m_bBehindPlayerDesired;
    bool          m_bCamLookingAtVector;
    bool          m_bCollisionChecksOn;
    bool          m_bFixingBeta;
    bool          m_bTheHeightFixerVehicleIsATrain;
    bool          m_bLookBehindCamWasInFront;
    bool          m_bLookingBehind;
    bool          m_bLookingLeft;
    bool          m_bLookingRight;
    bool          m_bResetStatics;
    bool          m_bResetKeyboardStatics;
    bool          m_bRotating;
    eCamMode      m_nMode;
    unsigned int  m_nFinishTime;
    unsigned int  m_nDoCollisionChecksOnFrameNum;
    unsigned int  m_nDoCollisionCheckEveryNumOfFrames;
    unsigned int  m_nFrameNumWereAt;
    unsigned int  m_nDirectionWasLooking;
    float         m_fSyphonModeTargetZOffSet;
    float         m_fAlphaSpeedOverOneFrame;
    float         m_fBetaSpeedOverOneFrame;
    float         m_fCamBufferedHeight;
    float         m_fCamBufferedHeightSpeed;
    float         m_fCloseInPedHeightOffset;
    float         m_fCloseInPedHeightOffsetSpeed;
    float         m_fCloseInCarHeightOffset;
    float         m_fCloseInCarHeightOffsetSpeed;
    float         m_fDimensionOfHighestNearCar;
    float         m_fDistanceBeforeChanges;
    float         m_fFovSpeedOverOneFrame;
    float         m_fMinDistAwayFromCamWhenInterPolating;
    float         m_fPedBetweenCameraHeightOffset;
    float         m_fPlayerInFrontSyphonAngleOffSet;
    float         m_fRadiusForDead;
    float         m_fRealGroundDist;
    float         m_fTimeElapsedFloat;
    float         m_fTilt;
    float         m_fTiltSpeed;
    float         m_fTransitionBeta;
    float         m_fTrueBeta;
    float         m_fTrueAlpha;
    float         m_fInitialPlayerOrientation;
    float         m_fVerticalAngle;
    float         m_fAlphaSpeed;
    float         m_fFOV;
    float         m_fFOVSpeed;
    float         m_fHorizontalAngle;
    float         m_fBetaSpeed;
    float         m_fDistance;
    float         m_fDistanceSpeed;
    float         m_fCaMinDistance;
    float         m_fCaMaxDistance;
    float         m_fSpeedVar;
    float         m_fCameraHeightMultiplier;
    float         m_fTargetZoomGroundOne;
    float         m_fTargetZoomGroundTwo;
    float         m_fTargetZoomGroundThree;
    float         m_fTargetZoomOneZExtra;
    float         m_fTargetZoomTwoZExtra;
    float         m_fTargetZoomTwoInteriorZExtra;
    float         m_fTargetZoomThreeZExtra;
    float         m_fTargetZoomZCloseIn;
    float         m_fMinRealGroundDist;
    float         m_fTargetCloseInDist;
    float         m_fBeta_Targeting;
    float         m_fX_Targetting;
    float         m_fY_Targetting;
    i32           m_pCarWeAreFocussingOn;
    float         m_pCarWeAreFocussingOnI;
    float         m_fCamBumpedHorz;
    float         m_fCamBumpedVert;
    i32           m_nCamBumpedTime;
    CVector       m_vecSourceSpeedOverOneFrame;
    CVector       m_vecTargetSpeedOverOneFrame;
    CVector       m_vecUpOverOneFrame;
    CVector       m_vecTargetCoorsForFudgeInter;
    CVector       m_vecCamFixedModeVector;
    CVector       m_vecCamFixedModeSource;
    CVector       m_vecCamFixedModeUpOffSet;
    CVector       m_vecLastAboveWaterCamPosition;
    CVector       m_vecBufferedPlayerBodyOffset;
    CVector       m_vecFront;
    CVector       m_vecSource;
    CVector       m_vecSourceBeforeLookBehind;
    CVector       m_vecUp;
    CVector       m_avecPreviousVectors[2];
    CVector       m_avecTargetHistoryPos[4];
    unsigned int  m_anTargetHistoryTime[4];
    unsigned int  m_nCurrentHistoryPoints;
    CEntity      *m_pCamTargetEntity;
    float         m_fCameraDistance;
    float         m_fIdealAlpha;
    float         m_fPlayerVelocity;
    CVehicle     *m_pLastCarEntered;
    CPed         *m_pLastPedLookedAt;
    bool          m_bFirstPersonRunAboutActive;
DECL_CLASS_END()
CHECKSIZE(CCam, 0x210, 0x228);

#endif // __AML_PSDK_SACAM_H