#ifndef AVBL_H
#define AVBL_H

#include <QHash>
#include <QList>
#include <QString>

class QDomDocument;
class QDomElement;
class BVHNode;
class TimelineTrail;
class TrailItem;


/** This is not a data class for AVBL files. Rather it's just helper for them (save/load).
    It's methods are supposed to be called in 'static' manner. */

class Avbl
{
public:
  Avbl();

  /** Returns TRUE on success */
  bool SaveToFile(QList<TimelineTrail*> trails, QString fileName);
  QList<TrailItem*>* LoadFromFile(QString fileName);

  bool HasErrors() const { return hasErrors; }
  QString ErrorMessage() const { return errorMessage; }


private:
  bool hasErrors;
  QString errorMessage;

  QList<TrailItem*>* linkLoadedItems(TrailItem*** sortedItems, int trailsCount);
  void createLimbWeightsElement(QDomDocument document, QDomElement parentElement,
                                BVHNode* limb, int frames);
  void loadLimbWeights(BVHNode* limb, QHash<QString, QDomElement>* bones);
};

#endif // AVBL_H
