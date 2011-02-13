#include "Avbl.h"
#include "bvh.h"
#include "TimelineTrail.h"
#include "TrailItem.cpp"
#include "WeightedAnimation.h"
#include <QTextStream>
#include <QtXml/QDomCDATASection>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>



Avbl::Avbl() { }


bool Avbl::SaveToFile(QList<TimelineTrail*> trails, QString fileName)
{
  hasErrors = false;
  errorMessage = "";

  QDomDocument document;
  QDomProcessingInstruction pi =
    document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
  document.appendChild(pi);
  QDomElement rootElm = document.createElement("avbl");
  QDomElement trailsElm = document.createElement("trailsDescription");
  trailsElm.setAttribute("count", trails.size());
  rootElm.appendChild(trailsElm);

  for(int trail=0; trail<trails.size(); trail++)
  {
    TrailItem* currentItem = trails.at(trail)->firstItem();
    int orderOnTrail = 0;     //zero-based position index denoting order of the item on its trail
                              //it's here to ease trail's items re-linking later when parsing

    while(currentItem!=0)
    {
      QDomElement animElm = document.createElement("animation");
      animElm.setAttribute("name", currentItem->Name);
      animElm.setAttribute("trail", trail);
      animElm.setAttribute("trailOrder", orderOnTrail);
      animElm.setAttribute("position", currentItem->beginIndex());

      QDomElement bvhElm = document.createElement("bvhData");
      QString bvhData;
      QTextStream outStream(&bvhData, QIODevice::WriteOnly);
      outStream << endl;
      BVH temp;
      temp.bvhWriteToTextStream(currentItem->getAnimation(), outStream);
      outStream.flush();
      QDomCDATASection cData = document.createCDATASection(bvhData);
      //TODO: whole CDATA approach is rather temporary. Should be heavy 'objective' (in XML way)
      bvhElm.appendChild(cData);
      animElm.appendChild(bvhElm);

      QDomElement fWeightsElm = document.createElement("frameWeights");
      for(int i=0; i<currentItem->frames(); i++)
      {
        QDomElement fWeight = document.createElement("frame");
        fWeight.setAttribute("number", i);
        fWeight.setAttribute("weight", currentItem->getWeight(i));
        fWeightsElm.appendChild(fWeight);
      }
      animElm.appendChild(fWeightsElm);

      QDomElement bWeightsElm = document.createElement("boneWeights");
      BVHNode* root = currentItem->getAnimation()->getMotion();
      createBoneWeightsElement(document, bWeightsElm, root, currentItem->frames());
      animElm.appendChild(bWeightsElm);

      rootElm.appendChild(animElm);
      document.appendChild(rootElm);

      currentItem = currentItem->nextItem();
      orderOnTrail++;
    }

    QDomElement trailElm = document.createElement("trail");
    trailElm.setAttribute("order", trail);
    trailElm.setAttribute("itemsCount", orderOnTrail);
    trailsElm.appendChild(trailElm);
  }

  QString fileText = document.toString(4);
  QFile file(fileName);
  file.open(QFile::WriteOnly);
  QTextStream out(&file);
  out.setRealNumberPrecision(4);

  out << fileText;
  file.close();

  return true;    //success. TODO: a branch for fail (to write data)
}


QList<TrailItem*> Avbl::LoadFromFile(QString fileName)
{
  hasErrors = false;
  errorMessage = "";
  QDomDocument document;
  QFile file(fileName);

  QList<TrailItem*> result;

  if (!file.open(QIODevice::ReadOnly))
    throw new QString("I/O exception: Can't open file " + fileName);
  if (!document.setContent(&file))      //TODO: the method can do much more to describe error
  {
    hasErrors = true;
    errorMessage = "Error parsing input document";
    file.close();
    throw new QString("XML exception: Error parsing XML file" + fileName);
  }
  file.close();

  QDomElement avbl = document.documentElement().firstChildElement("avbl");
  QDomElement trailsDesc = avbl.elementsByTagName("trailsDescription").at(0).toElement();
  int trailsCount = trailsDesc.attribute("count", "3").toInt();
  TrailItem*** loadedItems = new TrailItem**[trailsCount];          //uaaaa
  QDomNodeList trails = trailsDesc.elementsByTagName("trail");
  for(int t=0; t<trailsCount; t++)
  {
    int numItems = trails.at(t).toElement().attribute("itemsCount", "0").toInt();
    if(numItems>0)
      loadedItems[t] = new TrailItem*[numItems+1];    //it's "+1" to make place for an end mark

    loadedItems[numItems] = 0;                        //end mark
  }

  QDomNodeList items = avbl.elementsByTagName("animation");
  for(int i=0; i<items.size(); i++)
  {
    QDomElement itemElm = items.at(i).toElement();
    QDomNode bvhData = itemElm.elementsByTagName("bvhData").at(0);
    QDomCDATASection cData = bvhData.firstChild().toCDATASection();
    QString bvh = cData.data();
    BVH* b = new BVH();
    BVHNode* _what_ = b->bvhReadFromString(bvh);
    WeightedAnimation* wa = new WeightedAnimation(b, "");     //Is in 'b' really what WeightedAnimation needs? TODO: resolve.
    QString name = itemElm.attribute("name", "--unknown--");
    int trailIndex = itemElm.attribute("trail", "-1").toInt();
    int beginIndex = itemElm.attribute("position", "-1").toInt();
    int trailOrder = itemElm.attribute("trailOrder", "-1").toInt();

    TrailItem* tempItem = new TrailItem(wa, name, beginIndex, false);
    loadedItems[trailIndex][trailOrder] = tempItem;
  }

  return result;
}


/** @param sortedItems - first dimension are trails. Second dimensions are TrailItems inside trails. Those
                         are pointers (third "dimension"). The length of second dimension is delimited
                         with NULL (0) mark on the end (after last TrailItem) */
QList<TrailItem*> Avbl::linkLoadedItems(TrailItem*** sortedItems, int trailsCount)
{
  for(int trail=0; trail<trailsCount; trail++)
  {
    for(int item=0; ; item++)
    {
      if(item==0)
        sortedItems[trail][item]->setPreviousItem(0);     //_firstItem

      if(sortedItems[trail][item]==0)
        break;      //we hit end mark

      sortedItems[trail][item]->setNextItem(sortedItems[trail][item+1]);
      if(sortedItems[trail][item+1]!=0)
        sortedItems[trail][item+1]->setPreviousItem(sortedItems[trail][item]);
    }
  }
}


void Avbl::createBoneWeightsElement(QDomDocument document, QDomElement parentElement,
                                    BVHNode* limb, int frames)
{
  QDomElement limbElm = document.createElement("bone");
  limbElm.setAttribute("name", limb->name());

  for(int i=0; i<frames; i++)
  {
    QDomElement bWeight = document.createElement("frame");
    bWeight.setAttribute("number", i);
    bWeight.setAttribute("weight", 123456);         //TODO: real limb weights
    limbElm.appendChild(bWeight);
  }

  parentElement.appendChild(limbElm);

  for(int x=0; x<limb->numChildren(); x++)
    createBoneWeightsElement(document, parentElement, limb->child(x), frames);
}
