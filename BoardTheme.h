#ifndef BOARDTHEME_H
#define BOARDTHEME_H

#include <QColor>
typedef class BoardTheme{

public:
    struct SquareColours {
        QColor normal;
        QColor legal;
        QColor selected;
        QColor moveFromHighlight;
        QColor moveToHighlight;
    };
    SquareColours lightSquares = { QColor("#F0D9B5"), QColor("#AEEA84"), QColor("#FFD700"), QColor("#1E90FF"), QColor("#FFA500") };
    SquareColours darkSquares = { QColor("#B58863"), QColor("#9A6324"), QColor("#FFD700"), QColor("#1E90FF"), QColor("#FFA500") };
}Theme;

//    QColor lightSquareCol = QColor("#F0D9B5"), darkSquareCol = QColor("#B58863"), lastFromCol = QColor("#1E90FF"), LastToCol = QColor("#FFA500");
//QColor SelectedSquareCol = QColor("#FFA500");

#endif // BOARDTHEME_H
