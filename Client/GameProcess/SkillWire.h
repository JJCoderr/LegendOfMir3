#pragma once
namespace SkillWire {
constexpr WORD DeleteMessage=212;
constexpr int RecordSize=74,EncodedRecordSize=99,MaximumRecords=255,ShortcutCount=12;
constexpr int KeyOffset=4,IdOffset=6,CastingDelayOffset=49,ManaThresholdOffset=53;
constexpr BYTE EncodingFirst=0x3c,EncodingLast=0x7b;
}
