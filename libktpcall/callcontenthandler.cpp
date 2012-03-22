/*
    Copyright (C) 2011 Collabora Ltd. <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "callcontenthandler_p.h"
#include "sourcecontrollers.h"
#include "sinkcontrollers_p.h"
#include <QGlib/Connect>
#include <QGst/Pad>
#include <QGst/ElementFactory>
#include <KDebug>

//BEGIN PendingCallContentHandler

PendingCallContentHandler::PendingCallContentHandler(const Tp::CallChannelPtr & callChannel,
                                                     const QTf::ContentPtr & tfContent,
                                                     const QGst::PipelinePtr & pipeline,
                                                     QObject *parent)
    : QObject(parent)
{
    m_contentHandler = new CallContentHandler(parent);
    m_contentHandler->d->init(tfContent, pipeline);

    m_callChannel = callChannel;
    m_tfContent = tfContent;
    QTimer::singleShot(0, this, SLOT(findCallContent()));
}

void PendingCallContentHandler::findCallContent()
{
    Tp::CallContents callContents = m_callChannel->contents();
    Q_FOREACH (const Tp::CallContentPtr & callContent, callContents) {
        //FIXME telepathy-farstream should provide an object path here
        if (callContent->type() == m_tfContent->property("media-type").toInt()) {
            m_contentHandler->d->setCallContent(callContent);
            Q_EMIT ready(m_tfContent, m_contentHandler);
            deleteLater();
            return;
        }
    }

    kDebug() << "CallContent not found. Waiting for tp-qt4 to synchronize with d-bus.";
    connect(m_callChannel.data(), SIGNAL(contentAdded(Tp::CallContentPtr)),
            SLOT(onContentAdded(Tp::CallContentPtr)));
}

void PendingCallContentHandler::onContentAdded(const Tp::CallContentPtr & callContent)
{
    //FIXME telepathy-farstream should provide an object path here
    if (callContent->type() == m_tfContent->property("media-type").toInt()) {
        m_contentHandler->d->setCallContent(callContent);
        Q_EMIT ready(m_tfContent, m_contentHandler);
        deleteLater();
    }
}

//END PendingCallContentHandler
//BEGIN CallContentHandlerPrivate

CallContentHandlerPrivate::~CallContentHandlerPrivate()
{
    kDebug();
    m_sourceControllerPad->unlink(m_queue->getStaticPad("sink"));
    m_queue->getStaticPad("src")->unlink(m_tfContent->property("sink-pad").get<QGst::PadPtr>());
    m_queue->setState(QGst::StateNull);
    m_sourceController->releaseSrcPad(m_sourceControllerPad);
    m_sinkManager->unlinkAllPads();
}

void CallContentHandlerPrivate::init(const QTf::ContentPtr & tfContent,
                                     const QGst::PipelinePtr & pipeline)
{
    kDebug();

    m_tfContent = tfContent;
    m_pipeline = pipeline;
    QGlib::connect(tfContent, "src-pad-added", this, &CallContentHandlerPrivate::onSrcPadAdded);
    QGlib::connect(tfContent, "start-sending", this, &CallContentHandlerPrivate::onStartSending);
    QGlib::connect(tfContent, "stop-sending", this, &CallContentHandlerPrivate::onStopSending);

    switch(tfContent->property("media-type").toInt()) {
    case Tp::MediaStreamTypeAudio:
        m_sourceController = new AudioSourceController(pipeline, this);
        m_sinkManager = new AudioSinkManager(pipeline, this);
        break;
    case Tp::MediaStreamTypeVideo:
        m_sourceController = new VideoSourceController(pipeline, this);
        m_sinkManager = new VideoSinkManager(pipeline, this);
        break;
    default:
        Q_ASSERT(false);
    }

    m_queue = QGst::ElementFactory::make("queue");
    m_pipeline->add(m_queue);
    m_queue->syncStateWithParent();

    m_sourceControllerPad = m_sourceController->requestSrcPad();
    m_sourceControllerPad->link(m_queue->getStaticPad("sink"));
    m_queue->getStaticPad("src")->link(m_tfContent->property("sink-pad").get<QGst::PadPtr>());

    connect(m_sinkManager, SIGNAL(controllerCreated(BaseSinkController*)),
            this, SLOT(onControllerCreated(BaseSinkController*)));
    connect(m_sinkManager, SIGNAL(controllerDestroyed(BaseSinkController*)),
            this, SLOT(onControllerDestroyed(BaseSinkController*)));
}

void CallContentHandlerPrivate::setCallContent(const Tp::CallContentPtr & callContent)
{
    kDebug();
    m_callContent = callContent;

    //the sink manager needs the content to discover contacts
    m_sinkManager->setCallContent(callContent);
}

void CallContentHandlerPrivate::onSrcPadAdded(uint contactHandle,
                                              const QGlib::ObjectPtr & fsStream,
                                              const QGst::PadPtr & pad)
{
    Q_UNUSED(fsStream);
    m_sinkManager->handleNewSinkPad(contactHandle, pad);
}

bool CallContentHandlerPrivate::onStartSending()
{
    kDebug() << "Start sending requested";
    m_sourceController->setSourceEnabled(true);
    return true;
}

bool CallContentHandlerPrivate::onStopSending()
{
    kDebug() << "Stop sending requested";
    m_sourceController->setSourceEnabled(false);
    return true;
}

void CallContentHandlerPrivate::onControllerCreated(BaseSinkController *controller)
{
    kDebug() << "Sink controller created for" << controller->contact()->alias();
    m_sinkControllers.insert(controller);
    Q_EMIT q->sinkControllerAdded(controller);
}

void CallContentHandlerPrivate::onControllerDestroyed(BaseSinkController *controller)
{
    kDebug() << "Sink controller destroyed";
    m_sinkControllers.remove(controller);
    Q_EMIT q->sinkControllerRemoved(controller);
}

//END CallContentHandlerPrivate
//BEGIN CallContentHandler

CallContentHandler::CallContentHandler(QObject *parent)
    : QObject(parent), d(new CallContentHandlerPrivate(this))
{
}

CallContentHandler::~CallContentHandler()
{
    delete d;
}

Tp::CallContentPtr CallContentHandler::callContent() const
{
    return d->m_callContent;
}

BaseSourceController *CallContentHandler::sourceController() const
{
    return d->m_sourceController;
}

QSet<BaseSinkController*> CallContentHandler::sinkControllers() const
{
    return d->m_sinkControllers;
}


//END CallContentHandler

#include "callcontenthandler.moc"
#include "callcontenthandler_p.moc"