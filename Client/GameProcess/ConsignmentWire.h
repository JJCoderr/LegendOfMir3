#pragma once

// MIR3 455BF0 list decoding, 4546F0 drawing, 456334 listing identity.
namespace ConsignmentWire {
constexpr int Register=1130,NextPage=1131,PreviousPage=1132,Buy=1133,Refresh=1134,Cancel=1135,Search=1136;
constexpr int RegistrationNameLimit=27,RegistrationDescriptionLimit=123;
constexpr int ListMessage=1355,WaitingMessage=1400;
constexpr int RecordBytes=312,Server=0,ServerBytes=16,Id=16;
constexpr int Seller=20,SellerBytes=52,Price=72,Date=76,DateBytes=15;
constexpr int Item=91,ItemBytes=100,Description=191,DescriptionBytes=121;
}
