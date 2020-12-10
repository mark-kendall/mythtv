// Qt
#include <QDomDocument>
#include <QCryptographicHash>

// MythTV
#include "mythlogging.h"
#include "mythpainter.h"
#include "mythuiprocedural.h"

/*! \brief A procedural texture class.
 *
 * Data to provide to any shader; assuming projection already handled:-
 *
 *                  Type             Bytes          Notes
 *  Transform       16*float(4x4)    64
 *  Vertices        4*float          16
 *  Texcoords       4*float          16             Optional
 *  Alpha           float            4
 *  Seconds         uint             4              Seconds since epoch to prog start
 *  Milliseconds    uint             4              Centiseconds since prog start
 *  Visible         uint             4
 *  Activated       uint             4
 *  Deactivated     uint             4
 *  Hidden          uint             4
 *  Index           ushort           2
 *  Elements        ushort           2
 *  Total                            128
 *  Max Vulkan                       128
 *
 * Full millisecond accuracy requires 64bit - or 8bytes - but 64bit integer
 * support is limited.
 * Max 32bit int:        2,147,483,647
 * Centiseconds in day:      8,640,000 - wraps after 248.5 days
 * Milliseconds in day:     86,400,000 - wraps after 24.8 days
*/
MythUIProcedural::MythUIProcedural(MythUIType* Parent, const QString& Name)
  : MythUIType(Parent, Name)
{
}

void MythUIProcedural::DrawSelf(MythPainter* Painter, int XOffset, int YOffset,
                                int AlphaMod, QRect ClipRect)
{
    QRect area = GetArea();
    area.translate(XOffset, YOffset);
    Painter->SetClipRect(ClipRect);
    Painter->DrawProcedural(area, AlphaMod, m_source, m_hash);
}

bool MythUIProcedural::ParseElement(const QString& FileName, QDomElement& Element, bool ShowWarnings)
{
    if (Element.tagName() != "source")
        return MythUIType::ParseElement(FileName, Element, ShowWarnings);

    for (auto name = Element.firstChild(); !name.isNull(); name = name.nextSibling())
    {
        if (auto cdata = name.toCDATASection(); !cdata.isNull())
        {
            m_source = std::make_shared<QByteArray>(cdata.data().toLatin1().constData());
            m_hash = QCryptographicHash::hash(*m_source, QCryptographicHash::Md5);
            break;
        }
    }

    if (!m_source)
        LOG(VB_GENERAL, LOG_WARNING, "Failed to retrieve source code for procedural texture");
    return m_source != nullptr;
}

void MythUIProcedural::CopyFrom(MythUIType* Base)
{
    auto * proc = dynamic_cast<MythUIProcedural*>(Base);
    if (proc)
    {
        m_source = proc->m_source;
        m_hash   = proc->m_hash;
    }
    MythUIType::CopyFrom(Base);
}

void MythUIProcedural::CreateCopy(MythUIType* Parent)
{
    auto * proc = new MythUIProcedural(Parent, objectName());
    proc->CopyFrom(this);
}

void MythUIProcedural::Pulse()
{
    MythUIType::Pulse();
    SetRedraw();
}
