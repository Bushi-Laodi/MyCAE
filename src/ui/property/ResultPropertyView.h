#pragma once

class QWidget;
struct ResultObject;

class ResultPropertyView
{
public:
    static void populate(QWidget *parent, const ResultObject &resultObject);
};
