struct Camera {
    float4x4 view;
    float4x4 projection;
    float4x4 previous_view;
    float4x4 previous_projection;
};

/*!
 * \brief All the push constants that are available to a shader that uses the standard pipeline layout
 */
[[vk::push_constant]]
struct StandardPushConstants {
    /*!
     * \brief Index of the camera that will render this draw
     */
    uint camera_index;

    /*!
     * \brief Index of the material data for the current draw
     */
    uint material_index;
} constants;

/*!
 * \brief Array of all the materials 
 */
[[vk::binding(0, 0)]]
StructuredBuffer<Camera> cameras : register (t0);

/*!
 * \brief Array of all the materials 
 */
[[vk::binding(1, 0)]]
StructuredBuffer<MaterialData> material_buffer : register (t1);

/*!
 * \brief Point sampler you can use to sample any texture
 */
[[vk::binding(2, 0)]]
SamplerState point_sampler : register(s0, space0);

/*!
 * \brief Bilinear sampler you can use to sample any texture
 */
[[vk::binding(3, 0)]]
SamplerState bilinear_sampler : register(s0, space1);

/*!
 * \brief Trilinear sampler you can use to sample any texture
 */
[[vk::binding(4, 0)]]
SamplerState trilinear_sampler : register(s0, space2);

/*!
 * \brief Array of all the textures that are available for a shader to sample from
 */
[[vk::binding(5, 0)]]
Texture2D textures[] : register(t3);
