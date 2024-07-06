#include "aisettings.h"
#include <QSettings>

AISettings::AISettings(QObject *parent)
    : QObject(parent)
{
    // loadSettings();
}

void AISettings::requestAbortSearchSlot() {
    emit requestAbortSearch();
}
void AISettings::saveSettings() {
    QSettings settings("Sainadh", "chessgame");

    settings.beginGroup("AISettings");
    settings.setValue("depth", depth);
    settings.setValue("useIterativeDeepening", useIterativeDeepening);
    settings.setValue("useTranspositionTable", useTranspositionTable);
    settings.setValue("useThreading", useThreading);
    settings.setValue("useFixedDepthSearch", useFixedDepthSearch);
    settings.setValue("searchTimeMillis", searchTimeMillis);
    settings.setValue("endlessSearchMode", endlessSearchMode);
    settings.setValue("clearTTEachMove", clearTTEachMove);
    settings.setValue("useBook", useBook);
    settings.setValue("book", book);
    settings.setValue("maxBookPly", maxBookPly);
    // Assume PromotionMode is an enum and you store it as an int
    settings.setValue("promotionsToSearch", static_cast<int>(promotionsToSearch));
    settings.endGroup();
}

void AISettings::loadSettings() {
    QSettings settings("Sainadh", "chessgame");

    settings.beginGroup("AISettings");
    depth = settings.value("depth", 20).toInt();
    useIterativeDeepening = settings.value("useIterativeDeepening", true).toBool();
    useTranspositionTable = settings.value("useTranspositionTable", true).toBool();
    useThreading = settings.value("useThreading", true).toBool();
    useFixedDepthSearch = settings.value("useFixedDepthSearch", true).toBool();
    searchTimeMillis = settings.value("searchTimeMillis", 1000).toInt();
    endlessSearchMode = settings.value("endlessSearchMode", false).toBool();
    clearTTEachMove = settings.value("clearTTEachMove", false).toBool();
    useBook = settings.value("useBook", false).toBool();
    book = settings.value("book", "").toString();
    maxBookPly = settings.value("maxBookPly", 10).toInt();
    promotionsToSearch = static_cast<MoveGenerator::PromotionMode>(settings.value("promotionsToSearch", 0).toInt());
    settings.endGroup();
}
