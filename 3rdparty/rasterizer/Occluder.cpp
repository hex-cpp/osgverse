#include "Occluder.h"

#include "VectorMath.h"

#include <cassert>

std::unique_ptr<Occluder> Occluder::bake(const std::vector<__m128>& vertices, __m128 refMin, __m128 refMax)
{
  assert(vertices.size() % 16 == 0);

  // Simple k-means clustering by normal direction to improve backface culling efficiency
  std::vector<__m128> quadNormals;
  quadNormals.reserve(vertices.size());
  for (auto i = 0; i < vertices.size(); i += 4)
  {
    auto v0 = vertices[i + 0];
    auto v1 = vertices[i + 1];
    auto v2 = vertices[i + 2];
    auto v3 = vertices[i + 3];

    quadNormals.push_back(normalize(_mm_add_ps(normal(v0, v1, v2), normal(v0, v2, v3))));
  }

#if false
  std::vector<__m128> centroids;
  std::vector<uint32_t> centroidAssignment;
  centroids.push_back(_mm_setr_ps(+1.0f, 0.0f, 0.0f, 0.0f));
  centroids.push_back(_mm_setr_ps(0.0f, +1.0f, 0.0f, 0.0f));
  centroids.push_back(_mm_setr_ps(0.0f, 0.0f, +1.0f, 0.0f));
  centroids.push_back(_mm_setr_ps(0.0f, -1.0f, 0.0f, 0.0f));
  centroids.push_back(_mm_setr_ps(0.0f, 0.0f, -1.0f, 0.0f));
  centroids.push_back(_mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f));

  centroidAssignment.resize(vertices.size() / 4);
#else
  std::vector<__m128> centroids = {
    _mm_setr_ps(+1.0f, 0.0f, 0.0f, 0.0f),
    _mm_setr_ps(0.0f, +1.0f, 0.0f, 0.0f),
    _mm_setr_ps(0.0f, 0.0f, +1.0f, 0.0f),
    _mm_setr_ps(0.0f, -1.0f, 0.0f, 0.0f),
    _mm_setr_ps(0.0f, 0.0f, -1.0f, 0.0f),
    _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f),
  };
  std::vector<uint32_t> centroidAssignment(vertices.size() / 4);
#endif

  bool anyChanged = true;
  for (int iter = 0; iter < 10 && anyChanged; ++iter)
  {
    anyChanged = false;

    for (auto j = 0; j < quadNormals.size(); ++j)
    {
      __m128 normal = quadNormals[j];

      __m128 bestDistance = _mm_set1_ps(-std::numeric_limits<float>::infinity());
      uint32_t bestCentroid = 0;
      for (int k = 0; k < centroids.size(); ++k)
      {
        __m128 distance = _mm_dp_ps(centroids[k], normal, 0x7F);
        if (_mm_comige_ss(distance, bestDistance))
        {
          bestDistance = distance;
          bestCentroid = k;
        }
      }

      if (centroidAssignment[j] != bestCentroid)
      {
        centroidAssignment[j] = bestCentroid;
        anyChanged = true;
      }
    }

    for (int k = 0; k < centroids.size(); ++k)
    {
      centroids[k] = _mm_setzero_ps();
    }

    for (int j = 0; j < quadNormals.size(); ++j)
    {
      int k = centroidAssignment[j];

      centroids[k] = _mm_add_ps(centroids[k], quadNormals[j]);
    }

    for (int k = 0; k < centroids.size(); ++k)
    {
      centroids[k] = normalize(centroids[k]);
    }
  }

  std::vector<__m128> orderedVertices;
  orderedVertices.reserve(centroids.size() * vertices.size()); // worst case
  for (uint32_t k = 0; k < centroids.size(); ++k)
  {
    for (int j = 0; j < vertices.size() / 4; ++j)
    {
      if (centroidAssignment[j] == k)
      {
        orderedVertices.push_back(vertices[4 * j + 0]);
        orderedVertices.push_back(vertices[4 * j + 1]);
        orderedVertices.push_back(vertices[4 * j + 2]);
        orderedVertices.push_back(vertices[4 * j + 3]);
      }
    }
  }

  // pad to 32 elements so that the transformation loop does not break
  const uint32_t unpaddedSize = (uint32_t)orderedVertices.size();
  while (orderedVertices.size() % 32 != 0)
      orderedVertices.push_back(orderedVertices.back());

  auto occluder = std::make_unique<Occluder>();
  __m128 invExtents = _mm_div_ps(_mm_set1_ps(1.0f), _mm_sub_ps(refMax, refMin));

  __m128 scalingX = _mm_set1_ps(2047.0f);
  __m128 scalingY = _mm_set1_ps(2047.0f);
  __m128 scalingZ = _mm_set1_ps(1023.0f);

  __m128 half = _mm_set1_ps(0.5f);

  occluder->m_packetCount = 0;
  occluder->m_vertexData = reinterpret_cast<__m256i*>(_aligned_malloc(orderedVertices.size() * 4, 32));

  for (size_t i = 0; i < orderedVertices.size(); i += 32)
  {
	  __m128i v[8];

    for (auto j = 0; j < 4; ++j)
    {
      // Transform into [0,1] space relative to bounding box
      __m128 v0 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j +  0], refMin), invExtents);
      __m128 v1 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j +  4], refMin), invExtents);
      __m128 v2 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j +  8], refMin), invExtents);
      __m128 v3 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j + 12], refMin), invExtents);
	  __m128 v4 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j + 16], refMin), invExtents);
	  __m128 v5 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j + 20], refMin), invExtents);
	  __m128 v6 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j + 24], refMin), invExtents);
	  __m128 v7 = _mm_mul_ps(_mm_sub_ps(orderedVertices[i + j + 28], refMin), invExtents);

      // Transpose into [xxxx][yyyy][zzzz][wwww]
      _MM_TRANSPOSE4_PS(v0, v1, v2, v3);
	  _MM_TRANSPOSE4_PS(v4, v5, v6, v7);

      // Scale and truncate to int
      v0 = _mm_fmadd_ps(v0, scalingX, half);
      v1 = _mm_fmadd_ps(v1, scalingY, half);
      v2 = _mm_fmadd_ps(v2, scalingZ, half);

	  v4 = _mm_fmadd_ps(v4, scalingX, half);
	  v5 = _mm_fmadd_ps(v5, scalingY, half);
	  v6 = _mm_fmadd_ps(v6, scalingZ, half);

      __m128i X0 = _mm_sub_epi32(_mm_cvttps_epi32(v0), _mm_set1_epi32(1024));
      __m128i Y0 = _mm_cvttps_epi32(v1);
      __m128i Z0 = _mm_cvttps_epi32(v2);

	  __m128i X1 = _mm_sub_epi32(_mm_cvttps_epi32(v4), _mm_set1_epi32(1024));
	  __m128i Y1 = _mm_cvttps_epi32(v5);
	  __m128i Z1 = _mm_cvttps_epi32(v6);

      // Pack to 11/11/10 format
      __m128i XYZ0 = _mm_or_si128(_mm_slli_epi32(X0, 21), _mm_or_si128(_mm_slli_epi32(Y0, 10), Z0));
	  __m128i XYZ1 = _mm_or_si128(_mm_slli_epi32(X1, 21), _mm_or_si128(_mm_slli_epi32(Y1, 10), Z1));

	  v[2 * j + 0] = XYZ0;
	  v[2 * j + 1] = XYZ1;
    }

	occluder->m_vertexData[occluder->m_packetCount++] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(v + 0));
	occluder->m_vertexData[occluder->m_packetCount++] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(v + 2));
	occluder->m_vertexData[occluder->m_packetCount++] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(v + 4));
	occluder->m_vertexData[occluder->m_packetCount++] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(v + 6));
  }

  // truncate to the original unpadded size
  orderedVertices.resize(unpaddedSize);
  occluder->m_refMin = refMin;
  occluder->m_refMax = refMax;

  __m128 min = _mm_set1_ps(+std::numeric_limits<float>::infinity());
  __m128 max = _mm_set1_ps(-std::numeric_limits<float>::infinity());

  for (size_t i = 0; i < orderedVertices.size(); ++i)
  {
    min = _mm_min_ps(vertices[i], min);
    max = _mm_max_ps(vertices[i], max);
  }

  // Set W = 1 - this is expected by frustum culling code
  min = _mm_blend_ps(min, _mm_set1_ps(1.0f), 0b1000);
  max = _mm_blend_ps(max, _mm_set1_ps(1.0f), 0b1000);

  occluder->m_boundsMin = min;
  occluder->m_boundsMax = max;

  occluder->m_center = _mm_mul_ps(_mm_add_ps(max, min), _mm_set1_ps(0.5f));

  return occluder;
}

