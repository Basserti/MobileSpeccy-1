#include "keyboardwidget.h"
#include <QGridLayout>

#if defined(WIN32)
static const int s_button_positions[40][3]
{
  { 0,  0, 3 },
  { 0,  4, 3 },
  { 0,  8, 3 },
  { 0, 12, 3 },
  { 0, 16, 3 },
  { 0, 20, 3 },
  { 0, 24, 3 },
  { 0, 28, 3 },
  { 0, 32, 3 },
  { 0, 36, 3 },
  { 1,  1, 3 },
  { 1,  5, 3 },
  { 1,  9, 3 },
  { 1, 13, 3 },
  { 1, 17, 3 },
  { 1, 21, 3 },
  { 1, 25, 3 },
  { 1, 29, 3 },
  { 1, 33, 3 },
  { 1, 37, 3 },
  { 2,  2, 3 },
  { 2,  6, 3 },
  { 2, 10, 3 },
  { 2, 14, 3 },
  { 2, 18, 3 },
  { 2, 22, 3 },
  { 2, 26, 3 },
  { 2, 30, 3 },
  { 2, 34, 3 },
  { 2, 38, 3 },
  { 3,  0, 2 },
  { 3,  3, 3 },
  { 3,  7, 3 },
  { 3, 11, 3 },
  { 3, 15, 3 },
  { 3, 19, 3 },
  { 3, 23, 3 },
  { 3, 27, 3 },
  { 3, 31, 3 },
  { 3, 35, 6 },
};
#endif
#if defined (Q_OS_ANDROID)
static const int s_button_positions[40][3]
{
  { 0,  0, 2 },
  { 0,  2, 2 },
  { 0,  4, 2 },
  { 0,  6, 2 },
  { 0,  8, 2 },
  { 0, 10, 2 },
  { 0, 12, 2 },
  { 0, 14, 2 },
  { 0, 16, 2 },
  { 0, 18, 2 },
  { 1,  1, 2 },
  { 1,  3, 2 },
  { 1,  5, 2 },
  { 1,  7, 2 },
  { 1,  9, 2 },
  { 1, 11, 2 },
  { 1, 13, 2 },
  { 1, 15, 2 },
  { 1, 17, 2 },
  { 1, 19, 2 },
  { 2,  2, 2 },
  { 2,  4, 2 },
  { 2,  6, 2 },
  { 2,  8, 2 },
  { 2, 10, 2 },
  { 2, 12, 2 },
  { 2, 14, 2 },
  { 2, 16, 2 },
  { 2, 18, 2 },
  { 2, 20, 2 },
  { 3,  0, 1 },
  { 3,  1, 2 },
  { 3,  3, 2 },
  { 3,  5, 2 },
  { 3,  7, 2 },
  { 3,  9, 2 },
  { 3, 11, 2 },
  { 3, 13, 2 },
  { 3, 15, 2 },
  { 3, 17, 3 },
};
#endif


static int s_button_bits[40][2]
{
    {0, 11}, {1, 11}, {2, 11}, {3, 11}, {4, 11},
    {4, 12}, {3, 12}, {2, 12}, {1, 12}, {0, 12},
    {0, 10}, {1, 10}, {2, 10}, {3, 10}, {4, 10},
    {4, 13}, {3, 13}, {2, 13}, {1, 13}, {0, 13},
    {0,  9}, {1,  9}, {2,  9}, {3,  9}, {4,  9},
    {4, 14}, {3, 14}, {2, 14}, {1, 14}, {0, 14},
    {0,  8}, {1,  8}, {2,  8}, {3,  8}, {4,  8},
    {4, 15}, {3, 15}, {2, 15}, {1, 15}, {0, 15},
};

static QString s_button_labels[40]
{
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", "ENT",
    "CS", "Z", "X", "C", "V", "B", "N", "M", "SS", "SPC"
};

KeyboardWidget::KeyboardWidget(QWidget *parent) : QWidget(parent)
{
    QGridLayout * layout = new QGridLayout(this);
    setLayout(layout);
    for (int i = 0; i < 40; i++)
    {
        ZXPushButton * button = new ZXPushButton(
                    s_button_bits[i][0],
                    s_button_bits[i][1], this);

        if ((button->row() == 0 and button->col() == 8) or
            (button->row() == 1 and button->col() == 15))
        {
            button->setCheckable(true);
        }

        connect(button,
                SIGNAL(pressed(int,int)),
                this,
                SLOT(on_key_pressed(int,int)));
        connect(button,
                SIGNAL(released(int,int)),
                this,
                SLOT(on_key_released(int,int)));

        button->setText(s_button_labels[i]);
        layout->addWidget(button, s_button_positions[i][0],
                s_button_positions[i][1], 1,
                s_button_positions[i][2]);
        buttons.append(button);

    }
}

KeyboardWidget::~KeyboardWidget()
{
    qDeleteAll(buttons);
}

void KeyboardWidget::on_key_pressed(int row, int col)
{
    emit key_pressed(row, col);
}

void KeyboardWidget::on_key_released(int row, int col)
{
    emit key_released(row, col);
}
