#pragma once

struct ResponseBodyFilePath
{
    char domain[256] = {0};
    char response_body[4096] = {0};
    int statusCode = 0;
};

struct iTreeNodes
{
    int left_child = 0;
    int right_child = 0;
    int size = 0;
    bool is_leaf;
    int split_value;
};
