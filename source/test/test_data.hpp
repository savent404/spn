#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstring>

namespace test_data {
namespace dcp {
    /* TODO: Changed PC to capture this */
    const char kDcpAllSelector[] = "08000693cf32000c29ba09ea8892fefe05000100000100010004ffff00000000000000000000000000000000000000000000000000000000";
    const char kDcpIdentResp200smt[] = "e073e7a77f780813a9bc90408892feff0501000194ef00000060020500040000020702010011000053372d32303020534d4152542056330002020012000073372d3230302d736d6172742d303032020300060000002a0119020400040000030002070004000000640102000e0001c0a8023cffffff0000000000";
    const char kDcpIdentRespEcoPn[] = "e073e7a77f7830b851390be18892feff0501000194ef00000068020200110000657432303065636f706e2e64657637000102000e0001c0a80238ffffff0000000000020300060000002a030602040004000001000207000400000001020500120000020102020203020402050206010101020201000c0000455432303065636f504e";
    const char kDcpIdentRespX208[] = "000c29ba09ea08000693cf328892feff0501010000010000005e0205001c00000101010202010202020302040205033d0501050205030504ffff020100050000494e43000202000b0000583230382d424f524400020300060000002a0a0102040004000001000102000e0001c0a80006ffffff00c0a80001";
    const char kDcpSetReqX208[] = "08000693cf32000c29ba09ea8892fefd040001000001000000120102000e0001c0a8000affffff00c0a80001000000000000000000000000";
    const char kDcpSetRespX208[] = "000c29ba09ea08000693cf328892fefd0401010000010000000805040003010200000000000000000000000000000000000000000000000000000000";
    const char kDcpSignalSetReq[] = "040404040404e073e7a77f778892fefd0400000013ae000000080503000400000100000000000000000000000000000000000000000000000000000000000000";
    const char KDcpSignalSetResp[] = "e073e7a77f770404040404048892fefd0401000013ae0000000805040003050300000000000000000000000000000000000000000000000000000000";
    const char kDcpNameOfStationSetReq[] = "040404040404e073e7a77f778892fefd04000000421c0000001c020200120001657432303065636f706e2e646576323205020002000100000000000000000000";
    const char kDcpNameOfStationSetResp[] = "e073e7a77f770404040404048892fefd04010000421c0000001005040003020200000504000305020000000000000000000000000000000000000000";
    const char kDcpNameOfStationSetReqTemp[] = "040404040404e073e7a77f778892fefd040000006cbd0000001c020200120000657432303065636f706e2e646576323205020002000000000000000000000000";
    const char kDcpNameOfStationSetRespTemp[] = "e073e7a77f770404040404048892fefd040100006cbd0000001005040003020200000504000305020000000000000000000000000000000000000000";
    const char kDcpIpParamSetReq[] = "040404040404e073e7a77f778892fefd040000004e17000000120102000e0001c0a8023dffffff00000000000000000000000000000000000000000000000000";
    const char kDcpIpParamSetResp[] = "e073e7a77f770404040404048892fefd040100004e170000000805040003010200000000000000000000000000000000000000000000000000000000";
    const char kDcpResetFactorySetReq[] = "040404040404e073e7a77f778892fefd040000007c03000000060505000200000000000000000000000000000000000000000000000000000000000000000000";
    const char kDcpResetFactorySetResp[] = "040404040404e073e7a77f778892fefd040000007c03000000060505000200000000000000000000000000000000000000000000000000000000000000000000";
    const char kDcpSetReqStationWithStop[] = "30b8510fe348e073e7a77f788892fefd0400000003640000001002020006000164656d6f05020002000100000000000000000000000000000000000000000000";
    const char kDcpSetRespStationWithStop[] = "e073e7a77f7830b8510fe3488892fefd04010000036400000010050400030202000005040003050200002e64657635000102000e0001c0a80238ffff";
    struct DataParser {
        using value_type = std::vector<uint8_t>;
        using value_ptr = std::shared_ptr<value_type>;

        value_ptr operator()(const char* data)
        {
            value_ptr value = std::make_shared<value_type>();
            for (size_t i = 0; i < strlen(data); i += 2) {
                value->push_back(std::stoi(std::string(data + i, 2), nullptr, 16));
            }
            return value;
        }
    };
}
}
