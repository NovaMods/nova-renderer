#ifndef VULKAN_MOD_MC_ENTITY_H
#define VULKAN_MOD_MC_ENTITY_H

// FILE: C:/Users/David/Documents/MC/force-src/src/main/native/vulkan_mod/mc_entity.h

namespace vulkan_mod {
class store;
} /* End of namespace vulkan_mod */

namespace vulkan_mod {


    /** 
     *  Represents an entity from Minecraft.
     *  
     *  Entities from Minecraft have their position, rotation, animation states, damage states, and whatever else sent from Minecraft each frame
     */
class mc_entity {

    // Attributes
 public:
    int id;
    glm::vec3 position;
    glm::vec3 rotation;
    
    /** 
     *  Identifier to link the mc_entity instance to the entity model
     */
    int entity_type;
    bool is_loaded;
    bool should_show_damage;

    // Associations
 public:

    /**
     * @element-type store
     */
    store *mystore;

//end of class mc_entity
};

} /* End of namespace vulkan_mod */

#endif // VULKAN_MOD_MC_ENTITY_H
