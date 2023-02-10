#pragma once
#include <Application.hpp>
#include <Core/OpenGLUtils.hpp>
#include "Scene.hpp"
using namespace UniEngine;
namespace Galaxy
{
/// <summary>
/// The calculated precise position of the star.
/// </summary>
struct StarPosition : IDataComponent
{
    glm::dvec3 m_value;
};
struct SelectionStatus : IDataComponent
{
    int m_value;
};
/// <summary>
/// The seed of the star, use this to calculate initial position.
/// </summary>
struct StarInfo : IDataComponent
{
    bool m_initialized = false;
};

/// <summary>
/// Original color of the star
/// </summary>
struct OriginalColor : IDataComponent
{
    glm::vec3 m_value;
};
/// <summary>
/// The deviation of its orbit
/// </summary>
struct StarOrbitOffset : IDataComponent
{
    glm::dvec3 m_value;
};
/// <summary>
/// This will help calculate the orbit. Smaller = close to center, bigger = close to disk
/// </summary>
struct StarOrbitProportion : IDataComponent
{
    double m_value;
};
/// <summary>
/// This will help calculate the orbit. Smaller = close to center, bigger = close to disk
/// </summary>
struct SurfaceColor : IDataComponent
{
    glm::vec3 m_value;
    float m_intensity = 1.0f;
};
/// <summary>
/// The actual display color after selection system.
/// </summary>
struct DisplayColor : IDataComponent
{
    glm::vec3 m_value;
    float m_intensity = 1.0f;
};

struct StarOrbit : IDataComponent
{
    double m_a;
    double m_b;
    double m_speedMultiplier;

    double m_tiltX;
    double m_tiltY;
    double m_tiltZ;

    glm::dvec3 m_center;
    [[nodiscard]] glm::dvec3 GetPoint(
        const glm::dvec3 &orbitOffset, const double &time, const bool &isStar = true) const
    {
        const double angle = isStar ? time / glm::sqrt(m_a + m_b) * m_speedMultiplier : time;

        glm::dvec3 point{glm::sin(glm::radians(angle)) * m_a, 0, glm::cos(glm::radians(angle)) * m_b};

        point = Rotate(glm::angleAxis(glm::radians(m_tiltX), glm::dvec3(1, 0, 0)), point);
        point = Rotate(glm::angleAxis(glm::radians(m_tiltY), glm::dvec3(0, 1, 0)), point);
        point = Rotate(glm::angleAxis(glm::radians(m_tiltZ), glm::dvec3(0, 0, 1)), point);

        point += m_center;
        point += orbitOffset;
        return point;
    }
    static glm::dvec3 Rotate(const glm::qua<double> &rotation, const glm::dvec3 &point)
    {
        const double x = rotation.x * 2.0;
        const double y = rotation.y * 2.0;
        const double z = rotation.z * 2.0;
        const double xx = rotation.x * x;
        const double yy = rotation.y * y;
        const double zz = rotation.z * z;
        const double xy = rotation.x * y;
        const double xz = rotation.x * z;
        const double yz = rotation.y * z;
        const double wx = rotation.w * x;
        const double wy = rotation.w * y;
        const double wz = rotation.w * z;
        glm::dvec3 res;
        res.x = (1.0 - (yy + zz)) * point.x + (xy - wz) * point.y + (xz + wy) * point.z;
        res.y = (1.0 - (xx + zz)) * point.y + (yz - wx) * point.z + (xy + wz) * point.x;
        res.z = (1.0 - (xx + yy)) * point.z + (xz - wy) * point.x + (yz + wx) * point.y;
        return res;
    }
};
/// <summary>
/// The star cluster it actually belongs to.
/// </summary>
struct StarClusterIndex : IDataComponent
{
    int m_value = 0;
};

class StarClusterPattern
{
    double m_diskA = 0;
    double m_diskB = 0;
    double m_coreA = 0;
    double m_coreB = 0;
    double m_centerA = 0;
    double m_centerB = 0;
    double m_coreDiameter = 0;

  public:
    std::string m_name = "Cluster Pattern";
    StarClusterIndex m_starClusterIndex;
    void OnInspect();
    double m_ySpread = 0.05;
    double m_xzSpread = 0.015;

    double m_diskDiameter = 3000;
    double m_diskEccentricity = 0.5;

    double m_coreProportion = 0.4;
    double m_coreEccentricity = 0.7;

    double m_centerDiameter = 10;
    double m_centerEccentricity = 0.3;

    double m_diskSpeed = 1;
    double m_coreSpeed = 5;
    double m_centerSpeed = 10;

    double m_diskTiltX = 0;
    double m_diskTiltZ = 0;
    double m_coreTiltX = 0;
    double m_coreTiltZ = 0;
    double m_centerTiltX = 0;
    double m_centerTiltZ = 0;

    float m_diskEmissionIntensity = 3.0f;
    float m_coreEmissionIntensity = 2.0f;
    float m_centerEmissionIntensity = 1.0f;
    glm::vec3 m_diskColor = glm::vec3(0, 0, 1);
    glm::vec3 m_coreColor = glm::vec3(1, 1, 0);
    glm::vec3 m_centerColor = glm::vec3(1, 1, 1);

    double m_twist = 360;
    glm::dvec3 m_centerOffset = glm::dvec3(0);
    glm::dvec3 m_centerPosition = glm::dvec3(0);

    void Apply(const bool &forceUpdateAllStars = false, const bool &onlyUpdateColors = false);

    void SetAb()
    {
        m_diskA = m_diskDiameter * m_diskEccentricity;
        m_diskB = m_diskDiameter * (1 - m_diskEccentricity);
        m_centerA = m_centerDiameter * m_centerEccentricity;
        m_centerB = m_centerDiameter * (1 - m_centerEccentricity);
        m_coreDiameter = m_centerDiameter / 2 + m_centerDiameter / 2 +
                         (m_diskA + m_diskB - m_centerDiameter / 2 - m_centerDiameter / 2) * m_coreProportion;
        m_coreA = m_coreDiameter * m_coreEccentricity;
        m_coreB = m_coreDiameter * (1 - m_coreEccentricity);
    }

    /// <summary>
    /// Set the ellipse by the proportion.
    /// </summary>
    /// <param name="starOrbitProportion">
    /// The position of the ellipse in the density waves, range is from 0 to 1
    /// </param>
    /// <param name="orbit">
    /// The ellipse will be reset by the proportion and the density wave properties.
    /// </param>
    [[nodiscard]] StarOrbit GetOrbit(const double &starOrbitProportion) const
    {
        StarOrbit orbit;
        if (starOrbitProportion > m_coreProportion)
        {
            // If the wave is outside the disk;
            const double actualProportion = (starOrbitProportion - m_coreProportion) / (1 - m_coreProportion);
            orbit.m_a = m_coreA + (m_diskA - m_coreA) * actualProportion;
            orbit.m_b = m_coreB + (m_diskB - m_coreB) * actualProportion;
            orbit.m_tiltX = m_coreTiltX - (m_coreTiltX - m_diskTiltX) * actualProportion;
            orbit.m_tiltZ = m_coreTiltZ - (m_coreTiltZ - m_diskTiltZ) * actualProportion;
            orbit.m_speedMultiplier = m_coreSpeed + (m_diskSpeed - m_coreSpeed) * actualProportion;
        }
        else
        {
            const double actualProportion = starOrbitProportion / m_coreProportion;
            orbit.m_a = m_centerA + (m_coreA - m_centerA) * actualProportion;
            orbit.m_b = m_centerB + (m_coreB - m_centerB) * actualProportion;
            orbit.m_tiltX = m_centerTiltX - (m_centerTiltX - m_coreTiltX) * actualProportion;
            orbit.m_tiltZ = m_centerTiltZ - (m_centerTiltZ - m_coreTiltZ) * actualProportion;
            orbit.m_speedMultiplier = m_centerSpeed + (m_coreSpeed - m_centerSpeed) * actualProportion;
        }
        orbit.m_tiltY = -m_twist * starOrbitProportion;
        orbit.m_center = m_centerOffset * (1 - starOrbitProportion) + m_centerPosition;
        return orbit;
    }

    [[nodiscard]] StarOrbitOffset GetOrbitOffset(const double &proportion) const
    {
        double offset = glm::sqrt(1 - proportion);
        StarOrbitOffset orbitOffset;
        glm::dvec3 d3;
        d3.y = glm::gaussRand(0.0, 1.0) * (m_diskA + m_diskB) * m_ySpread;
        d3.x = glm::gaussRand(0.0, 1.0) * (m_diskA + m_diskB) * m_xzSpread;
        d3.z = glm::gaussRand(0.0, 1.0) * (m_diskA + m_diskB) * m_xzSpread;
        orbitOffset.m_value = d3;
        return orbitOffset;
    }

    [[nodiscard]] glm::vec3 GetColor(const double &proportion) const
    {
        glm::vec3 color = glm::vec3();
        if (proportion > m_coreProportion)
        {
            // If the wave is outside the disk;
            const double actualProportion = (proportion - m_coreProportion) / (1 - m_coreProportion);
            color = m_coreColor * (1 - static_cast<float>(actualProportion)) +
                    m_diskColor * static_cast<float>(actualProportion);
        }
        else
        {
            const double actualProportion = proportion / m_coreProportion;
            color = m_coreColor * static_cast<float>(actualProportion) +
                    m_centerColor * (1 - static_cast<float>(actualProportion));
        }
        return color;
    }

    [[nodiscard]] float GetIntensity(const double &proportion) const
    {
        float intensity = 1.0f;
        if (proportion > m_coreProportion)
        {
            // If the wave is outside the disk;
            const double actualProportion = (proportion - m_coreProportion) / (1 - m_coreProportion);
            intensity = m_coreEmissionIntensity * (1 - static_cast<float>(actualProportion)) +
                        m_diskEmissionIntensity * static_cast<float>(actualProportion);
        }
        else
        {
            const double actualProportion = proportion / m_coreProportion;
            intensity = m_coreEmissionIntensity * static_cast<float>(actualProportion) +
                        m_centerEmissionIntensity * (1 - static_cast<float>(actualProportion));
        }
        return intensity;
    }
};

class StarClusterSystem : public ISystem
{
    EntityRef m_rendererFront;
    EntityRef m_rendererBack;
    EntityQuery m_starQuery;
    EntityArchetype m_starArchetype;
    std::vector<StarClusterPattern> m_starClusterPatterns;
    bool m_useFront = true;
    int m_counter = 0;

    float m_applyPositionTimer = 0;
    float m_copyPositionTimer = 0;
    float m_calcPositionTimer = 0;
    float m_calcPositionResult = 0;
    float m_speed = 0.0f;
    float m_size = 0.05f;
    float m_galaxyTime = 0.0;
    bool m_firstTime = true;


  public:
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void OnInspect() override;

    void CalculateStarPositionSync();
    void ApplyPosition();
    void CopyPosition(const bool &reverse = false);
    void OnCreate() override;
    void Start() override;
    void Update() override;
    void PushStars(StarClusterPattern &pattern, const size_t &amount = 10000);
    void RandomlyRemoveStars(const size_t &amount = 10000);
    void ClearAllStars();
    void FixedUpdate() override;
    void OnEnable() override;

};
} // namespace Galaxy
