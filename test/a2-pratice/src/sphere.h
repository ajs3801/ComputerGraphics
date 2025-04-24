#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "cgmath.h"
#include "cgut.h"
#include <vector>

// create_sphere_mesh()
// - n_long: longitude (��: 72)
// - n_lat: latitude (��: 36)
// ��ȯ��: mesh* (�������� �Ҵ�� �� �޽�)
// ���� ������ (0,0,0), �������� 1�� �����ϸ�, ����, �븻, �ؽ�ó ��ǥ�� ����Ͽ� index buffer�� �ﰢ������ �����մϴ�.
inline mesh* create_sphere_mesh(int n_long, int n_lat)
{
    mesh* m = new mesh;

    // ���� �迭 ����
    // latitude: 0 ~ ��, longitude: 0 ~ 2�� (���� ���� �ߺ�)
    for (int lat = 0; lat <= n_lat; lat++) {
        float theta = (float(lat) / n_lat) * PI;  // 0 ~ ��
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        for (int lon = 0; lon <= n_long; lon++) {
            float phi = (float(lon) / n_long) * 2.0f * PI;  // 0 ~ 2��
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            vertex v;
            // ���� ���� ��ǥ (������ 1)
            v.pos = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
            // �븻 ���� (�� �߽ɿ����� ����; �̹� ���� ����)
            v.norm = v.pos.normalize();
            // �ؽ�ó ��ǥ: Tx = ��/(2��), Ty = 1 - ��/��
            v.tex = vec2(phi / (2.0f * PI), 1.0f - theta / PI);
            m->vertex_list.push_back(v);
        }
    }

    // �ε��� �迭 ����
    // (n_long+1) x (n_lat+1) ���ڿ��� �� �簢�� ��ġ�� �� ���� �ﰢ������ ������.
    for (int lat = 0; lat < n_lat; lat++) {
        for (int lon = 0; lon < n_long; lon++) {
            int first = lat * (n_long + 1) + lon;
            int second = first + n_long + 1;
            // �ﰢ�� 1
            m->index_list.push_back(first);
            m->index_list.push_back(second);
            m->index_list.push_back(first + 1);
            // �ﰢ�� 2
            m->index_list.push_back(second);
            m->index_list.push_back(second + 1);
            m->index_list.push_back(first + 1);
        }
    }

    // GPU ���� ���� �� ä�� (VBO, IBO, VAO)
    glGenBuffers(1, &m->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, m->vertex_list.size() * sizeof(vertex),
        m->vertex_list.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_list.size() * sizeof(uint),
        m->index_list.data(), GL_STATIC_DRAW);

    // cg_create_vertex_array()�� cgut.h�� �����Ǿ� ����
    m->vertex_array = cg_create_vertex_array(m->vertex_buffer, m->index_buffer);

    return m;
}

#endif
