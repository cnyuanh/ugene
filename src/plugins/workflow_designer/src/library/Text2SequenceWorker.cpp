#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/FailTask.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Gui/SeqPasterWidgetController.h>
#include "Text2SequenceWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString Text2SequenceWorkerFactory::ACTOR_ID("convert-text-to-sequence");

static const Descriptor TEXT_2_SEQUENCE_IN_TYPE_ID("text-2-sequence-in-type");
static const Descriptor TEXT_2_SEQUENCE_OUT_TYPE_ID("text-2-sequence-out-type");

static const QString SEQ_NAME_ATTR_ID("sequence-name");
static const QString ALPHABET_ATTR_ID("alphabet");
static const QString SKIP_SYM_ATTR_ID("skip-unknown");
static const QString REPLACE_SYM_ATTR_ID("replace-unknown-with");

static const QString SEQ_NAME_ATTR_DEF_VAL("Sequence");
static const QString ALPHABET_ATTR_ID_DEF_VAL("Auto");

/*******************************
 * Text2SequenceWorker
 *******************************/
QMap<QString, QString> Text2SequenceWorker::cuteAlIdNames = Text2SequenceWorker::initCuteAlNames();

QMap<QString, QString> Text2SequenceWorker::initCuteAlNames() {
    QMap<QString, QString> res;
    res[BaseDNAAlphabetIds::RAW()] = "All symbols";
    res[BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()] = "Standard DNA";
    res[BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()] = "Standard RNA";
    res[BaseDNAAlphabetIds::NUCL_DNA_EXTENDED()] = "Extended DNA";
    res[BaseDNAAlphabetIds::NUCL_RNA_EXTENDED()] = "Extended RNA";
    res[BaseDNAAlphabetIds::AMINO_DEFAULT()] = "Standard amino";
    return res;
}

void Text2SequenceWorker::init() {
    txtPort = ports.value(BasePorts::IN_TEXT_PORT_ID());
    outSeqPort = ports.value(BasePorts::OUT_SEQ_PORT_ID());
}

bool Text2SequenceWorker::isReady() {
    return txtPort->hasMessage();
}

Task * Text2SequenceWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(txtPort);
    QString seqName = actor->getParameter(SEQ_NAME_ATTR_ID)->getAttributeValue<QString>();
    if(seqName.isEmpty()) {
        return new FailTask(tr("Sequence name not set"));
    }
    if(tickedNum++ > 0) {
        seqName += QString::number(tickedNum);
    }
    QString alId = actor->getParameter(ALPHABET_ATTR_ID)->getAttributeValue<QString>();
    if(alId.isEmpty()) {
        alId = ALPHABET_ATTR_ID_DEF_VAL;
    } else {
        alId = cuteAlIdNames.key(alId, alId);
    }
    bool skipUnknown = actor->getParameter(SKIP_SYM_ATTR_ID)->getAttributeValue<bool>();
    QChar replaceChar;
    if(!skipUnknown) {
        QString replaceStr = actor->getParameter(REPLACE_SYM_ATTR_ID)->getAttributeValue<QString>();
        assert(replaceStr.size() <= 1);
        if(replaceStr.isEmpty()) {
            return new FailTask(tr("skip flag should be set or replace character defined"));
        }
        replaceChar = replaceStr.at(0);
    }
    QByteArray txt = inputMessage.getData().toMap().value(BaseSlots::TEXT_SLOT().getId()).value<QString>().toUtf8();
    
    DNAAlphabetRegistry * alphabetRegistry = AppContext::getDNAAlphabetRegistry();
    DNAAlphabet * alphabet = NULL;
    if(alId == ALPHABET_ATTR_ID_DEF_VAL) {
        alphabet = alphabetRegistry->findAlphabet(txt);
    } else {
        alphabet = alphabetRegistry->findById(alId);
    }
    if(alphabet == NULL) {
        QString msg;
        if(alId == ALPHABET_ATTR_ID_DEF_VAL) {
            msg = tr("Alphabet cannot be automatically detected");
        } else {
            msg = tr("Alphabet '%1' cannot be found");
        }
        return new FailTask(msg);
    }
    
    QByteArray normSequence = SeqPasterWidgetController::getNormSequence(alphabet, txt, !skipUnknown, replaceChar);
    DNASequence result(seqName, normSequence, alphabet);
    QVariantMap msgData;
    {
        msgData[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<DNASequence>(result);
    }
    if(outSeqPort) {
        outSeqPort->put(Message(BaseTypes::DNA_SEQUENCE_TYPE(), msgData));
    }
    if(txtPort->isEnded()) {
        outSeqPort->setEnded();
    }
    
    return NULL;
}

bool Text2SequenceWorker::isDone() {
    return txtPort->isEnded();
}

void Text2SequenceWorker::cleanup() {
}

/*******************************
 * Text2SequenceWorkerFactory
 *******************************/
void Text2SequenceWorkerFactory::init() {
    // ports description
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::TEXT_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr inSet(new MapDataType(TEXT_2_SEQUENCE_IN_TYPE_ID, inM));
        Descriptor inPortDesc(BasePorts::IN_TEXT_PORT_ID(), Text2SequenceWorker::tr("Input text"), 
            Text2SequenceWorker::tr("A text which will be converted to sequence"));
        portDescs << new PortDescriptor(inPortDesc, inSet, true);
        
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        DataTypePtr outSet(new MapDataType(TEXT_2_SEQUENCE_OUT_TYPE_ID, outM));
        Descriptor outPortDesc(BasePorts::OUT_SEQ_PORT_ID(), Text2SequenceWorker::tr("Output sequence"), 
            Text2SequenceWorker::tr("Converted sequence"));
        portDescs << new PortDescriptor(outPortDesc, outSet, false);
    }
    // attributes description
    QList<Attribute*> attrs;
    {
        Descriptor seqNameDesc(SEQ_NAME_ATTR_ID, Text2SequenceWorker::tr("Sequence name"), Text2SequenceWorker::tr("Result sequence name"));
        Descriptor alphabetDesc(ALPHABET_ATTR_ID, Text2SequenceWorker::tr("Sequence alphabet"), 
            Text2SequenceWorker::tr("Select one of the listed alphabets or choose auto to auto-detect"));
        Descriptor skipSymbolsDesc(SKIP_SYM_ATTR_ID, Text2SequenceWorker::tr("Skip unknown symbols"), 
            Text2SequenceWorker::tr("Do not include symbols that are not contained in alphabet"));
        Descriptor replaceSymbolsDesc(REPLACE_SYM_ATTR_ID, Text2SequenceWorker::tr("Replace unknown symbols with"),
            Text2SequenceWorker::tr("Replace unknown symbols with given character"));
        
        attrs << new Attribute(seqNameDesc, BaseTypes::STRING_TYPE(), /* required */ true, QVariant(SEQ_NAME_ATTR_DEF_VAL));
        attrs << new Attribute(alphabetDesc, BaseTypes::STRING_TYPE(), false, QVariant(ALPHABET_ATTR_ID_DEF_VAL));
        attrs << new Attribute(skipSymbolsDesc, BaseTypes::BOOL_TYPE(), false, QVariant(true));
        attrs << new Attribute(replaceSymbolsDesc, BaseTypes::STRING_TYPE(), false);
    }
    
    Descriptor protoDesc(Text2SequenceWorkerFactory::ACTOR_ID, 
        Text2SequenceWorker::tr("Convert text to sequence"), 
        Text2SequenceWorker::tr("Converts input text to sequence"));
    ActorPrototype * proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    
    // proto delegates
    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap alMap;
        QList<DNAAlphabet*> alps = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
        foreach(DNAAlphabet *a, alps){
            alMap[a->getName()] = Text2SequenceWorker::cuteAlIdNames[a->getId()];
        }
        alMap[ALPHABET_ATTR_ID_DEF_VAL] = ALPHABET_ATTR_ID_DEF_VAL;
        delegates[ALPHABET_ATTR_ID] = new ComboBoxDelegate(alMap);
        
        delegates[REPLACE_SYM_ATTR_ID] = new CharacterDelegate();
    }
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new Text2SequencePrompter());
    
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CONVERTERS(), proto);
    WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID )->registerEntry( new Text2SequenceWorkerFactory() );
}

Worker * Text2SequenceWorkerFactory::createWorker(Actor* a) {
    return new Text2SequenceWorker(a);
}

/*******************************
 * Text2SequencePrompter
 *******************************/
QString Text2SequencePrompter::composeRichDoc() {
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    IntegralBusPort * input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_TEXT_PORT_ID()));
    Actor * txtProducer = input->getProducer(BaseSlots::TEXT_SLOT().getId());
    QString txtProducetStr = tr(" from <u>%1</u>").arg(txtProducer ? txtProducer->getLabel() : unsetStr);
    
    QString seqName = getParameter(SEQ_NAME_ATTR_ID).value<QString>();
    QString seqNameStr = tr("sequence with name <u>%1</u>").arg(!seqName.isEmpty() ? seqName : unsetStr);
    
    QString alId = getParameter(ALPHABET_ATTR_ID).value<QString>();
    QString seqAlStr;
    if(alId == ALPHABET_ATTR_ID_DEF_VAL) {
        seqAlStr = tr("Automatically detect sequence alphabet");
    } else {
        alId = Text2SequenceWorker::cuteAlIdNames.key(alId, "");
        DNAAlphabet * alphabet = AppContext::getDNAAlphabetRegistry()->findById(alId);
        seqAlStr = tr("Set sequence alphabet to <u>%1</u>").arg(alphabet ? alphabet->getName() : unsetStr);
    }
    
    bool skipUnknown = getParameter(SKIP_SYM_ATTR_ID).value<bool>();
    QString replaceStr = getParameter(REPLACE_SYM_ATTR_ID).value<QString>();
    QString unknownSymbolsStr = skipUnknown ? tr("<u>skipped</u>") : 
                                              tr("<u>replaced with symbol %1</u>").arg(!replaceStr.isEmpty() ? replaceStr : unsetStr );
    
    QString doc = tr("Convert input text%1 to %2. %3. Unknown symbols are %4.")
        .arg(txtProducetStr)
        .arg(seqNameStr)
        .arg(seqAlStr)
        .arg(unknownSymbolsStr);
    return doc;
}

} // LocalWorkflow
} // U2
