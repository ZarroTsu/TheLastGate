#pragma once

typedef struct {
    float x;
    float y;
} Vec2;

static Vec2 vec2(float x, float y) { return (Vec2){x, y}; }

static int min(int a, int b) { return a < b ? a : b; }
static int max(int a, int b) { return a > b ? a : b; }
