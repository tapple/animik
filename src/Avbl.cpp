#include "Avbl.h"
#include "TimelineTrail.h"
#include "TrailItem.cpp"
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

  for(int trail=0; trail<trails.size(); trail++)
  {
    TrailItem* currentItem = trails.at(trail)->firstItem();

    while(currentItem!=0)
    {
      QDomElement animElm = document.createElement("animation");
      animElm.setAttribute("name", currentItem->Name);
      animElm.setAttribute("trail", trail);
      animElm.setAttribute("frame", currentItem->beginIndex());

      QDomElement bvhElm = document.createElement("bvhData");
      QDomCDATASection cData = document.createCDATASection("TODO: BVH does it directly to a file :( Must adjust it to get a QString");
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
    }
  }

  QString fileText = document.toString(4);
  QFile f(fileName);
  f.open(QFile::WriteOnly);
  QTextStream out(&f);
  out.setRealNumberPrecision(4);

  out << fileText;
  f.close();

  return true;    //success. TODO: a branch for fail (to write data)
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
