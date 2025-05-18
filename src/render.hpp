#ifndef RENDER_H
#define RENDER_H

struct RenderState
{
    bool sRGB;
    bool bloom;
    bool useNormal;
    bool useShadows;
    bool hdr;
    bool devMode;
    float exposure;
};

// Init Render State
RenderState renderState =  {
    .sRGB = true,
    .bloom = true,
    .useNormal = true,
    .useShadows = true,
    .hdr = true,
    .devMode = false,
    .exposure = 1.0f,
};

#endif
