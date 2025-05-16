#ifndef RENDER_H
#define RENDER_H

struct RenderState
{
    bool sRGB;
    bool bloom;
    bool useNormal;
    bool useShadows;
    bool hdr;
    bool grabMouse;
    float exposure;
};

#endif
