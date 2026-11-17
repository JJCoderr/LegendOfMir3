#pragma once
// MIR3 42F5A0 / Godot protocol/actor_ability.hpp. Wire order differs from ACTORABILITY.
namespace AbilityWire {
constexpr int Size=63,EncodedSize=84,ElementCount=7;
constexpr int Level=0,Defence=1,MagicDefence=3,Attack=5,Magic=7,Spirit=9;
constexpr int Health=11,Mana=13,MaxHealth=15,MaxMana=17,Experience=19,NextExperience=23;
constexpr int Weight=27,MaxWeight=29,Wear=31,MaxWear=32,Hand=33,MaxHand=34;
constexpr int Extension=35,ExtensionSize=28,AttackElements=36,StrengthElements=49,WeaknessElements=56;
constexpr BYTE MaximumProfession=2;
}
