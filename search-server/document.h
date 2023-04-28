#pragma once

#include <iostream>

struct Document
{
    Document() = default;
    Document(int init_id, double init_relevance, int init_rating) :
    id(init_id), relevance(init_relevance), rating(init_rating)
    {

    }
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

std::ostream& operator<<(std::ostream& out, const Document& doc);