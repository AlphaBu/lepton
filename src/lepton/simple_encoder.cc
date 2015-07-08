/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <string.h>
#include "bitops.hh"
#include "component_info.hh"
#include "uncompressed_components.hh"
#include "jpgcoder.hh"
#include "simple_encoder.hh"
#include "../io/SwitchableCompression.hh"
#include <algorithm>

unsigned int get_cmp(int cur_read_batch[4], int target[4]);
SimpleComponentEncoder::SimpleComponentEncoder() {
    memset(target, 0, sizeof(target));
    memset(cur_read_batch, 0, sizeof(cur_read_batch));
}
CodingReturnValue SimpleComponentEncoder::encode_chunk(const UncompressedComponents *colldata,
                                                       Sirikata::
                                                       SwitchableCompressionWriter<Sirikata::
                                                                                   DecoderCompressionWriter> *str_out) {
    // read coefficient data from file
    unsigned int batch_size = 1600;

    char zero[sizeof(target)] = {0};
    if (memcmp(target, zero, sizeof(target)) == 0) {
        unsigned int t24 = 65536 * 256;
        unsigned char bs[4] = {(unsigned char)(batch_size & 0xff), (unsigned char)((batch_size / 256) & 0xff),
                               (unsigned char)((batch_size / 65536) & 0xff), (unsigned char)((batch_size / t24) & 0xff)};
        str_out->Write(bs, sizeof(bs));        
        for (unsigned int cmp = 0; cmp < 4; ++cmp) {
            target[cmp] = colldata->component_size_in_blocks(cmp);
        }
    }
    unsigned int cmp = get_cmp(cur_read_batch, target);
    if (cmp == sizeof(cur_read_batch)/sizeof(cur_read_batch[0]) || cur_read_batch[cmp] == target[cmp]) {
        return CODING_DONE;
    }
    const signed short * start = colldata->full_component_nosync( cmp );
    while (cur_read_batch[cmp] < target[cmp]) {
        int cur_write_size = std::min((int)batch_size, target[cmp] - cur_read_batch[cmp]);
        str_out->Write(reinterpret_cast<const unsigned char*>(start + cur_read_batch[cmp] * 64) , sizeof( short ) * 64 * cur_write_size);
        cur_read_batch[cmp] += cur_write_size;        
        return CODING_PARTIAL;
    }
    assert(false && "UNREACHABLE");
    return CODING_PARTIAL;
}

SimpleComponentEncoder::~SimpleComponentEncoder() {

}