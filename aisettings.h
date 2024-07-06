#ifndef AISETTINGS_H
#define AISETTINGS_H

#include <QObject>
#include <QString>
#include <QFile>
#include "movegenerator.h"
#include "searchdiagnostics.h"

class Search;

class AISettings : public QObject
{
    Q_OBJECT

public:
    explicit AISettings(QObject *parent = nullptr);
    int depth = 5;
    bool useIterativeDeepening=true;
    bool useTranspositionTable=false;

    bool useThreading=false;
    bool useFixedDepthSearch=true;
    int searchTimeMillis = 1000;
    bool endlessSearchMode=false;
    bool clearTTEachMove=false;

    bool useBook=false;
    QString book;
    int maxBookPly = 10;

    MoveGenerator::PromotionMode promotionsToSearch;

    SearchDiagnostics diagnostics;
    void saveSettings();
    void loadSettings();
    void requestAbortSearchSlot();

signals:
    void requestAbortSearch();
};

#endif // AISETTINGS_H
