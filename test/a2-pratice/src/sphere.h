#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "cgmath.h"
#include "cgut.h"
#include <vector>

// create_sphere_mesh()
// - n_long: longitude (예: 72)
// - n_lat: latitude (예: 36)
// 반환값: mesh* (동적으로 할당된 구 메시)
// 구의 원점은 (0,0,0), 반지름은 1로 설정하며, 정점, 노말, 텍스처 좌표를 계산하여 index buffer로 삼각형들을 구성합니다.
inline mesh* create_sphere_mesh(int n_long, int n_lat)
{
    mesh* m = new mesh;

    // 정점 배열 생성
    // latitude: 0 ~ π, longitude: 0 ~ 2π (양쪽 끝은 중복)
    for (int lat = 0; lat <= n_lat; lat++) {
        float theta = (float(lat) / n_lat) * PI;  // 0 ~ π
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        for (int lon = 0; lon <= n_long; lon++) {
            float phi = (float(lon) / n_long) * 2.0f * PI;  // 0 ~ 2π
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            vertex v;
            // 구의 정점 좌표 (반지름 1)
            v.pos = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
            // 노말 벡터 (구 중심에서의 방향; 이미 단위 벡터)
            v.norm = v.pos.normalize();
            // 텍스처 좌표: Tx = φ/(2π), Ty = 1 - θ/π
            v.tex = vec2(phi / (2.0f * PI), 1.0f - theta / PI);
            m->vertex_list.push_back(v);
        }
    }

    // 인덱스 배열 생성
    // (n_long+1) x (n_lat+1) 격자에서 각 사각형 패치를 두 개의 삼각형으로 분할함.
    for (int lat = 0; lat < n_lat; lat++) {
        for (int lon = 0; lon < n_long; lon++) {
            int first = lat * (n_long + 1) + lon;
            int second = first + n_long + 1;
            // 삼각형 1
            m->index_list.push_back(first);
            m->index_list.push_back(second);
            m->index_list.push_back(first + 1);
            // 삼각형 2
            m->index_list.push_back(second);
            m->index_list.push_back(second + 1);
            m->index_list.push_back(first + 1);
        }
    }

    // GPU 버퍼 생성 및 채움 (VBO, IBO, VAO)
    glGenBuffers(1, &m->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, m->vertex_list.size() * sizeof(vertex),
        m->vertex_list.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_list.size() * sizeof(uint),
        m->index_list.data(), GL_STATIC_DRAW);

    // cg_create_vertex_array()는 cgut.h에 구현되어 있음
    m->vertex_array = cg_create_vertex_array(m->vertex_buffer, m->index_buffer);

    return m;
}

#endif
