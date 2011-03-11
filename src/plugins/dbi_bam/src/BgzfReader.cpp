#include "BAMDbiPlugin.h"
#include "IOException.h"
#include "InvalidFormatException.h"
#include "BgzfReader.h"

namespace U2 {
namespace BAM {

BgzfReader::BgzfReader(IOAdapter &ioAdapter):
    ioAdapter(ioAdapter),
    headerOffset(ioAdapter.bytesRead()),
    endOfFile(false)
{
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.next_in = Z_NULL;
    stream.avail_in = 0;
    stream.next_out = Z_NULL;
    stream.avail_out = 0;
    if(Z_OK != inflateInit2(&stream, 16 + 15)) {
        throw Exception(BAMDbiPlugin::tr("can't initialize zlib"));
    }
}

BgzfReader::~BgzfReader() {
    inflateEnd(&stream);
}

qint64 BgzfReader::read(char *buff, qint64 maxSize) {
    if(0 == maxSize) {
        return 0;
    }
    stream.next_out = (Bytef *)buff;
    stream.avail_out = maxSize;
    while(stream.avail_out > 0) {
        if(0 == stream.avail_in) {
            qint64 returnedValue = ioAdapter.readBlock(buffer, sizeof(buffer));
            if(-1 == returnedValue) {
                throw IOException(BAMDbiPlugin::tr("can't read input"));
            } else if(0 == returnedValue) {
                endOfFile = true;
                break;
            } else {
                stream.avail_in = returnedValue;
                stream.next_in = (Bytef *)buffer;
            }
        }
        int returnedValue = inflate(&stream, Z_SYNC_FLUSH);
        if(Z_STREAM_END == returnedValue) {
            nextBlock();
        } else if(Z_OK != returnedValue) {
            throw InvalidFormatException(BAMDbiPlugin::tr("can't decompress data"));
        }
    }
    if(0 == stream.avail_in) {
        nextBlock();
    }
    qint64 bytesRead = maxSize - stream.avail_out;
    return bytesRead;
}

qint64 BgzfReader::skip(qint64 size) {
    char skipBuffer[1024];
    qint64 bytesSkipped = 0;
    while(bytesSkipped < size) {
        qint64 toRead = qMin((qint64)sizeof(skipBuffer), size - bytesSkipped);
        qint64 returnedValue = read(skipBuffer, toRead);
        bytesSkipped += returnedValue;
        if(returnedValue < toRead) {
            break;
        }
    }
    return bytesSkipped;
}

bool BgzfReader::isEof()const {
    return endOfFile;
}

VirtualOffset BgzfReader::getOffset()const {
    return VirtualOffset(headerOffset, stream.total_out);
}

void BgzfReader::seek(VirtualOffset offset) {
    if((offset.getCoffset() == headerOffset) && (offset.getUoffset() >= (int)stream.total_out)) {
        qint64 toSkip = offset.getUoffset() - stream.total_out;
        if(skip(toSkip) < toSkip) {
            throw InvalidFormatException(BAMDbiPlugin::tr("unexpected end of file"));
        }
    } else {
        if(!ioAdapter.skip(offset.getCoffset() - ioAdapter.bytesRead())) {
            throw IOException(BAMDbiPlugin::tr("can't read input"));
        }
        stream.next_in = Z_NULL;
        stream.avail_in = 0;
        headerOffset = ioAdapter.bytesRead();
        inflateReset(&stream);
        if(skip(offset.getUoffset()) < offset.getUoffset()) {
            throw InvalidFormatException(BAMDbiPlugin::tr("unexpected end of file"));
        }
    }
    endOfFile = false;
}

void BgzfReader::nextBlock() {
    uInt oldAvailOut = stream.avail_out;
    stream.avail_out = 0;
    while(true) {
        if(0 == stream.avail_in) {
            qint64 returnedValue = ioAdapter.readBlock(buffer, sizeof(buffer));
            if(-1 == returnedValue) {
                throw IOException(BAMDbiPlugin::tr("can't read input"));
            } else if(0 == returnedValue) {
                endOfFile = true;
                break;
            } else {
                stream.avail_in = returnedValue;
                stream.next_in = (Bytef *)buffer;
            }
        }
        int returnedValue = inflate(&stream, Z_SYNC_FLUSH);
        if(Z_STREAM_END == returnedValue) {
            headerOffset = ioAdapter.bytesRead() - stream.avail_in;
            inflateReset(&stream);
        } else if(Z_BUF_ERROR == returnedValue) {
            break;
        } else if(Z_OK != returnedValue) {
            throw InvalidFormatException(BAMDbiPlugin::tr("can't decompress data"));
        }
    }
    stream.avail_out = oldAvailOut;
}

} // namespace BAM
} // namespace U2
