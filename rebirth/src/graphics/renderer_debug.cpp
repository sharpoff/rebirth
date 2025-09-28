#include <rebirth/graphics/renderer.h>

void Renderer::drawLine(vec3 p1, vec3 p2)
{
    debugDrawVertices.push_back(Vertex{.position = p1});
    debugDrawVertices.push_back(Vertex{.position = p2});
}

void Renderer::drawPlane(vec3 p1, vec3 p2, vec3 p3, vec3 p4)
{
    drawLine(p1, p2);
    drawLine(p2, p3);
    drawLine(p3, p4);
    drawLine(p4, p1);
}

void Renderer::drawBox(vec3 pos, vec3 halfExtent)
{
    // TODO:
}

void Renderer::drawSphere(vec3 pos, float radius)
{
    // TODO:
}