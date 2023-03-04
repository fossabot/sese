#include "sese/convert/Decompressor.h"
#include "zlib.h"

sese::Decompressor::Decompressor(sese::CompressionType type, size_t bufferSize) {
    stream = new z_stream;
    auto stm = (z_stream *) stream;
    stm->zalloc = nullptr;
    stm->zfree = nullptr;
    stm->opaque = nullptr;
    stm->avail_in = 0;
    stm->next_in = nullptr;
    stm->avail_out = 0;
    stm->next_out = nullptr;

    cap = bufferSize;
    buffer = new unsigned char[bufferSize];

    inflateInit2(stm, MAX_WBITS + (int) type);
}

sese::Decompressor::~Decompressor() {
    delete[] buffer;

    auto stm = (z_stream *) stream;
    ::inflateEnd(stm);
    delete stm;
    stream = nullptr;
}

void sese::Decompressor::input(const void *input, unsigned int inputLen) {
    auto stm = (z_stream *) stream;
    stm->avail_in = (unsigned int) inputLen;
    stm->next_in = (unsigned char *) input;
    stm->avail_out = (unsigned int) cap;
    stm->next_out = (unsigned char *) buffer;
}

int sese::Decompressor::inflate(sese::OutputStream *out) {
    auto stm = (z_stream *) stream;
    // 输出 buffer 未能完全输出，继续输出
    if (length != 0) {
        auto last = length - read;
        auto realWrote = out->write(buffer + read, last);
        if (last != realWrote) {
            // 输出 buffer 未能全部输出
            read += realWrote;
            return Z_BUF_ERROR;
        } else {
            length = 0;
            read = 0;

            stm->avail_out = (unsigned int) cap;
            stm->next_out = buffer;
        }
    }

    // 输入 buffer 未解析完全，继续解析
    int ret;
    do {
        ret = ::inflate(stm, Z_FINISH);
        if (ret == Z_STREAM_ERROR)
            return Z_STREAM_ERROR;

        auto wrote = (int) cap - stm->avail_out;
        auto realWrote = out->write(buffer, wrote);
        if (wrote != realWrote) {
            // 输出 buffer 未能全部输出
            length = wrote;
            read = realWrote;
            return Z_BUF_ERROR;
        } else {
            stm->avail_out = (unsigned int) cap;
            stm->next_out = buffer;
        }
    } while (ret != Z_STREAM_END);

    return Z_OK;
}

int sese::Decompressor::reset() {
    length = 0;
    read = 0;
    auto stm = (z_stream *) stream;
    return inflateReset(stm);
}

size_t sese::Decompressor::getTotalIn() const {
    auto stm = (z_stream *) stream;
    return stm->total_in;
}

size_t sese::Decompressor::getTotalOut() const {
    auto stm = (z_stream *) stream;
    return stm->total_out - length + read;
}