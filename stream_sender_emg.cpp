#include "stream_sender_emg.h"

StreamSenderEMG::StreamSenderEMG(QObject *parent) : QObject(parent)
{
    m_mmapFile.setFileName("E:\\COPY\\learn\\Course3\\sendfile\\MyAppDataMMF");
    if (!m_mmapFile.open(QIODevice::ReadWrite)) {
        qWarning() << "Cannot open mmap file:" << m_mmapFile.errorString();
        return;
    }
    if (!m_mmapFile.resize(MMAP_SIZE)) {
        qWarning() << "Cannot resize mmap file";
        return;
    }
    m_mappedData = m_mmapFile.map(0, MMAP_SIZE);
    if (!m_mappedData) {
        qWarning() << "Cannot map file to memory";
    }
}

StreamSenderEMG::~StreamSenderEMG()
{
    if (m_mappedData) {
        m_mmapFile.unmap(m_mappedData);
    }
    m_mmapFile.close();
}

void StreamSenderEMG::SendBuf(const QVector<uint16_t> &buf)
{
    if (!m_mappedData || buf.size() != 8) {
        return;
    }
    memcpy(m_mappedData, buf.constData(), 16);
}
