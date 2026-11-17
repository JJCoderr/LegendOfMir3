#pragma once
namespace MerchantService {
constexpr WORD RepairOk=669,RepairFail=670,StackSaleOk=1314,StackSaleFail=1315;
constexpr WORD DepositOk=701,StorageFull=702,DepositFail=703;
constexpr int Repair=1,Sell=2,Deposit=3;
constexpr int DepositedStack=1447,DepositedItem=1448,FullNotice=1449,DepositError=1450;
constexpr int SoldItem=1453,SoldStack=1454,SaleError=1455,Repaired=1456,RepairError=1457;
constexpr int DurabilityOffset=66,MaximumOffset=68;
}
