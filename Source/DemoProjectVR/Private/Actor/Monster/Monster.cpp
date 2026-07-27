#include "Actor/Monster/Monster.h"

#include "AI/MonsterAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AMonster::AMonster()
{
	AIControllerClass = AMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}
