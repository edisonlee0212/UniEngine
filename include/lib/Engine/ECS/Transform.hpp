#pragma once
#include <Entity.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
struct UNIENGINE_API GlobalTransformUpdateFlag : IDataComponent
{
    bool m_value = false;
};

struct UNIENGINE_API GlobalTransform : IDataComponent
{
    glm::mat4 m_value =
        glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    bool operator==(const GlobalTransform &other) const
    {
        return other.m_value == m_value;
    }
#pragma region Get &set
    bool Decompose(glm::vec3 &translation, glm::vec3 &eulerAngles, glm::vec3 &scale) const
    {
        using namespace glm;
        using T = float;
        mat4 LocalMatrix(m_value);

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
            return false;

        for (length_t i = 0; i < 4; ++i)
            for (length_t j = 0; j < 4; ++j)
                LocalMatrix[i][j] /= LocalMatrix[3][3];

        // perspectiveMatrix is used to solve for perspective, but it also provides
        // an easy way to test for singularity of the upper 3x3 component.
        mat4 PerspectiveMatrix(LocalMatrix);

        for (length_t i = 0; i < 3; i++)
            PerspectiveMatrix[i][3] = static_cast<T>(0);
        PerspectiveMatrix[3][3] = static_cast<T>(1);

        /// TODO: Fixme!
        if (epsilonEqual(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon<T>()))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        translation = vec3(LocalMatrix[3]);
        LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

        vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]); // v3Length(Row[0]);

        Row[0] = glm::detail::scale(Row[0], static_cast<T>(1));

        // Compute XY shear factor and make 2nd row orthogonal to 1st.
        glm::vec3 Skew;
        Skew.z = dot(Row[0], Row[1]);
        Row[1] = glm::detail::combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

        // Now, compute Y scale and normalize 2nd row.
        scale.y = length(Row[1]);
        Row[1] = glm::detail::scale(Row[1], static_cast<T>(1));
        Skew.z /= scale.y;

        // Compute XZ and YZ shears, orthogonalize 3rd row.
        Skew.y = glm::dot(Row[0], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
        Skew.x = glm::dot(Row[1], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

        // Next, get Z scale and normalize 3rd row.
        scale.z = length(Row[2]);
        Row[2] = glm::detail::scale(Row[2], static_cast<T>(1));
        Skew.y /= scale.z;
        Skew.x /= scale.z;

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }

        eulerAngles.y = glm::asin(-Row[0][2]);
        if (glm::cos(eulerAngles.y) != 0)
        {
            eulerAngles.x = atan2(Row[1][2], Row[2][2]);
            eulerAngles.z = atan2(Row[0][1], Row[0][0]);
        }
        else
        {
            eulerAngles.x = atan2(-Row[2][0], Row[1][1]);
            eulerAngles.z = 0;
        }
        return true;
    }
    bool Decompose(glm::vec3 &translation, glm::quat &rotation, glm::vec3 &scale) const
    {
        using namespace glm;
        using T = float;
        mat4 LocalMatrix(m_value);

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
            return false;

        for (length_t i = 0; i < 4; ++i)
            for (length_t j = 0; j < 4; ++j)
                LocalMatrix[i][j] /= LocalMatrix[3][3];

        // perspectiveMatrix is used to solve for perspective, but it also provides
        // an easy way to test for singularity of the upper 3x3 component.
        mat4 PerspectiveMatrix(LocalMatrix);

        for (length_t i = 0; i < 3; i++)
            PerspectiveMatrix[i][3] = static_cast<T>(0);
        PerspectiveMatrix[3][3] = static_cast<T>(1);

        /// TODO: Fixme!
        if (epsilonEqual(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon<T>()))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        translation = vec3(LocalMatrix[3]);
        LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

        vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]); // v3Length(Row[0]);

        Row[0] = glm::detail::scale(Row[0], static_cast<T>(1));

        // Compute XY shear factor and make 2nd row orthogonal to 1st.
        glm::vec3 Skew;
        Skew.z = dot(Row[0], Row[1]);
        Row[1] = glm::detail::combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

        // Now, compute Y scale and normalize 2nd row.
        scale.y = length(Row[1]);
        Row[1] = glm::detail::scale(Row[1], static_cast<T>(1));
        Skew.z /= scale.y;

        // Compute XZ and YZ shears, orthogonalize 3rd row.
        Skew.y = glm::dot(Row[0], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
        Skew.x = glm::dot(Row[1], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

        // Next, get Z scale and normalize 3rd row.
        scale.z = length(Row[2]);
        Row[2] = glm::detail::scale(Row[2], static_cast<T>(1));
        Skew.y /= scale.z;
        Skew.x /= scale.z;

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }
        int i, j, k = 0;
        T root, trace = Row[0].x + Row[1].y + Row[2].z;
        if (trace > static_cast<T>(0))
        {
            root = glm::sqrt(trace + static_cast<T>(1.0));
            rotation.w = static_cast<T>(0.5) * root;
            root = static_cast<T>(0.5) / root;
            rotation.x = root * (Row[1].z - Row[2].y);
            rotation.y = root * (Row[2].x - Row[0].z);
            rotation.z = root * (Row[0].y - Row[1].x);
        } // End if > 0
        else
        {
            static int Next[3] = {1, 2, 0};
            i = 0;
            if (Row[1].y > Row[0].x)
                i = 1;
            if (Row[2].z > Row[i][i])
                i = 2;
            j = Next[i];
            k = Next[j];

            root = glm::sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

            rotation[i] = static_cast<T>(0.5) * root;
            root = static_cast<T>(0.5) / root;
            rotation[j] = root * (Row[i][j] + Row[j][i]);
            rotation[k] = root * (Row[i][k] + Row[k][i]);
            rotation.w = root * (Row[j][k] - Row[k][j]);
        } // End if <= 0

        return true;
    }
    glm::vec3 GetPosition() const
    {
        return m_value[3];
    }
    glm::vec3 GetScale() const
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        return scale;
    }
    glm::quat GetRotation() const
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        return rotation;
    }
    glm::vec3 GetEulerRotation() const
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::vec3 rotation;
        Decompose(trans, rotation, scale);
        return rotation;
    }
    void SetPosition(const glm::vec3 &value)
    {
        m_value[3].x = value.x;
        m_value[3].y = value.y;
        m_value[3].z = value.z;
    }
    void SetScale(const glm::vec3 &value)
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        m_value = glm::translate(trans) * glm::mat4_cast(rotation) * glm::scale(value);
    }
    void SetRotation(const glm::quat &value)
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        m_value = glm::translate(trans) * glm::mat4_cast(value) * glm::scale(scale);
    }
    void SetEulerRotation(const glm::vec3 &value)
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        m_value = glm::translate(trans) * glm::mat4_cast(glm::quat(value)) * glm::scale(scale);
    }
    void SetValue(const glm::vec3 &position, const glm::vec3 &eulerRotation, const glm::vec3 &scale)
    {
        m_value = glm::translate(position) * glm::mat4_cast(glm::quat(eulerRotation)) * glm::scale(scale);
    }
    void SetValue(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale)
    {
        m_value = glm::translate(position) * glm::mat4_cast(rotation) * glm::scale(scale);
    }
#pragma endregion
};
struct UNIENGINE_API Transform : IDataComponent
{
    glm::mat4 m_value =
        glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    ;
    bool operator==(const Transform &other) const
    {
        return other.m_value == m_value;
    }
#pragma region Get &set
    bool Decompose(glm::vec3 &translation, glm::vec3 &eulerAngles, glm::vec3 &scale) const
    {
        using namespace glm;
        using T = float;
        mat4 LocalMatrix(m_value);

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
            return false;

        for (length_t i = 0; i < 4; ++i)
            for (length_t j = 0; j < 4; ++j)
                LocalMatrix[i][j] /= LocalMatrix[3][3];

        // perspectiveMatrix is used to solve for perspective, but it also provides
        // an easy way to test for singularity of the upper 3x3 component.
        mat4 PerspectiveMatrix(LocalMatrix);

        for (length_t i = 0; i < 3; i++)
            PerspectiveMatrix[i][3] = static_cast<T>(0);
        PerspectiveMatrix[3][3] = static_cast<T>(1);

        /// TODO: Fixme!
        if (epsilonEqual(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon<T>()))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        translation = vec3(LocalMatrix[3]);
        LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

        vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]); // v3Length(Row[0]);

        Row[0] = glm::detail::scale(Row[0], static_cast<T>(1));

        // Compute XY shear factor and make 2nd row orthogonal to 1st.
        glm::vec3 Skew;
        Skew.z = dot(Row[0], Row[1]);
        Row[1] = glm::detail::combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

        // Now, compute Y scale and normalize 2nd row.
        scale.y = length(Row[1]);
        Row[1] = glm::detail::scale(Row[1], static_cast<T>(1));
        Skew.z /= scale.y;

        // Compute XZ and YZ shears, orthogonalize 3rd row.
        Skew.y = glm::dot(Row[0], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
        Skew.x = glm::dot(Row[1], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

        // Next, get Z scale and normalize 3rd row.
        scale.z = length(Row[2]);
        Row[2] = glm::detail::scale(Row[2], static_cast<T>(1));
        Skew.y /= scale.z;
        Skew.x /= scale.z;

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }

        eulerAngles.y = glm::asin(-Row[0][2]);
        if (glm::cos(eulerAngles.y) != 0)
        {
            eulerAngles.x = atan2(Row[1][2], Row[2][2]);
            eulerAngles.z = atan2(Row[0][1], Row[0][0]);
        }
        else
        {
            eulerAngles.x = atan2(-Row[2][0], Row[1][1]);
            eulerAngles.z = 0;
        }
        return true;
    }
    bool Decompose(glm::vec3 &translation, glm::quat &rotation, glm::vec3 &scale) const
    {
        using namespace glm;
        using T = float;
        mat4 LocalMatrix(m_value);

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
            return false;

        for (length_t i = 0; i < 4; ++i)
            for (length_t j = 0; j < 4; ++j)
                LocalMatrix[i][j] /= LocalMatrix[3][3];

        // perspectiveMatrix is used to solve for perspective, but it also provides
        // an easy way to test for singularity of the upper 3x3 component.
        mat4 PerspectiveMatrix(LocalMatrix);

        for (length_t i = 0; i < 3; i++)
            PerspectiveMatrix[i][3] = static_cast<T>(0);
        PerspectiveMatrix[3][3] = static_cast<T>(1);

        /// TODO: Fixme!
        if (epsilonEqual(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon<T>()))
            return false;

        // First, isolate perspective.  This is the messiest.
        if (epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        translation = vec3(LocalMatrix[3]);
        LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

        vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]); // v3Length(Row[0]);

        Row[0] = glm::detail::scale(Row[0], static_cast<T>(1));

        // Compute XY shear factor and make 2nd row orthogonal to 1st.
        glm::vec3 Skew;
        Skew.z = dot(Row[0], Row[1]);
        Row[1] = glm::detail::combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

        // Now, compute Y scale and normalize 2nd row.
        scale.y = length(Row[1]);
        Row[1] = glm::detail::scale(Row[1], static_cast<T>(1));
        Skew.z /= scale.y;

        // Compute XZ and YZ shears, orthogonalize 3rd row.
        Skew.y = glm::dot(Row[0], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
        Skew.x = glm::dot(Row[1], Row[2]);
        Row[2] = glm::detail::combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

        // Next, get Z scale and normalize 3rd row.
        scale.z = length(Row[2]);
        Row[2] = glm::detail::scale(Row[2], static_cast<T>(1));
        Skew.y /= scale.z;
        Skew.x /= scale.z;

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }
        int i, j, k = 0;
        T root, trace = Row[0].x + Row[1].y + Row[2].z;
        if (trace > static_cast<T>(0))
        {
            root = glm::sqrt(trace + static_cast<T>(1.0));
            rotation.w = static_cast<T>(0.5) * root;
            root = static_cast<T>(0.5) / root;
            rotation.x = root * (Row[1].z - Row[2].y);
            rotation.y = root * (Row[2].x - Row[0].z);
            rotation.z = root * (Row[0].y - Row[1].x);
        } // End if > 0
        else
        {
            static int Next[3] = {1, 2, 0};
            i = 0;
            if (Row[1].y > Row[0].x)
                i = 1;
            if (Row[2].z > Row[i][i])
                i = 2;
            j = Next[i];
            k = Next[j];

            root = glm::sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

            rotation[i] = static_cast<T>(0.5) * root;
            root = static_cast<T>(0.5) / root;
            rotation[j] = root * (Row[i][j] + Row[j][i]);
            rotation[k] = root * (Row[i][k] + Row[k][i]);
            rotation.w = root * (Row[j][k] - Row[k][j]);
        } // End if <= 0

        return true;
    }
    glm::vec3 GetPosition() const
    {
        return m_value[3];
    }
    glm::vec3 GetScale() const
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        return scale;
    }
    glm::quat GetRotation() const
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        return rotation;
    }
    glm::vec3 GetEulerRotation() const
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::vec3 rotation;
        Decompose(trans, rotation, scale);
        return rotation;
    }
    void SetPosition(const glm::vec3 &value)
    {
        m_value[3].x = value.x;
        m_value[3].y = value.y;
        m_value[3].z = value.z;
    }
    void SetScale(const glm::vec3 &value)
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        m_value = glm::translate(trans) * glm::mat4_cast(rotation) * glm::scale(value);
    }
    void SetRotation(const glm::quat &value)
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        m_value = glm::translate(trans) * glm::mat4_cast(value) * glm::scale(scale);
    }
    void SetEulerRotation(const glm::vec3 &value)
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        Decompose(trans, rotation, scale);
        m_value = glm::translate(trans) * glm::mat4_cast(glm::quat(value)) * glm::scale(scale);
    }
    void SetValue(const glm::vec3 &position, const glm::vec3 &eulerRotation, const glm::vec3 &scale)
    {
        m_value = glm::translate(position) * glm::mat4_cast(glm::quat(eulerRotation)) * glm::scale(scale);
    }
    void SetValue(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale)
    {
        m_value = glm::translate(position) * glm::mat4_cast(rotation) * glm::scale(scale);
    }
#pragma endregion
};
} // namespace UniEngine