#ifndef SEARCHDIAGNOSTICS_H
#define SEARCHDIAGNOSTICS_H

#include <QString>
class SearchDiagnostics {
public:
    int lastCompletedDepth;
    QString moveVal;
    QString move;
    int eval;
    bool isBook;
    int numPositionsEvaluated;
};
#endif // SEARCHDIAGNOSTICS_H
