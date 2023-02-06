#include <LittleFS.h>
#include <unzipLIB.h>

void *zipOpen(const char *filename, int32_t *size)
{
    File *file = new File(LittleFS.open(filename, "r"));

    *size = (int32_t)file->size();

    return (void *)file;
}

void zipClose(void *p)
{
    ZIPFILE *pzf = (ZIPFILE *)p;
    File *f = (File *)pzf->fHandle;
    if (f)
        f->close();
}

int32_t zipRead(void *p, uint8_t *buffer, int32_t length)
{
    ZIPFILE *pzf = (ZIPFILE *)p;
    File *f = (File *)pzf->fHandle;
    int bytes = f->read(buffer, length);

    Serial.print("bytes read: ");
    Serial.println(bytes);

    return bytes;
}

int32_t zipSeek(void *p, int32_t position, int iType)
{
    ZIPFILE *pzf = (ZIPFILE *)p;
    File *f = (File *)pzf->fHandle;

    if (iType == SEEK_SET)
    {
        return f->seek(position);
    }
    else if (iType == SEEK_END)
    {
        return f->seek(position + pzf->iSize);
    }
    else
    { // SEEK_CUR
        long l = f->position();
        return f->seek(l + position);
    }
}
