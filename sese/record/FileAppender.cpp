#include "sese/record/FileAppender.h"

sese::record::FileAppender::FileAppender(io::FileStream::Ptr fileStream, sese::record::Level level)
    : AbstractAppender(level),
      fileStream(std::move(fileStream)) {
    bufferedStream = std::make_shared<io::BufferedStream>(this->fileStream, 4 * 1024);
}

sese::record::FileAppender::~FileAppender() noexcept {
    bufferedStream->flush();
    fileStream->close();
}

void sese::record::FileAppender::dump(const char *buffer, size_t size) noexcept {
    bufferedStream->write((void *) buffer, size);
}
