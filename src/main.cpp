#include "engine.hpp"
#include "components.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include "mesh.hpp"

int main()
{
    lavander::Engine engine{};

    try 
    {
        auto& reg = engine.getRegistry();
        auto e = reg.createEntity();

        reg.addComponent<lavander::Transform>(e,
            {
                glm::vec3(0.0f,0.0f,0.0f),
                glm::vec3(0.0f, glm::radians(0.0f), 0.0f),
                glm::vec3(0.5f, 0.5f, 0.5f)          
            
            });
        
        auto tex = std::make_shared<lavander::Texture2D>(engine.getDevice(), "../../src/assets/default_texture.png");

        tex->allocateDescriptor(engine.getMaterialDescriptorPool(), engine.getMaterialSetLayout());

        reg.addComponent<lavander::SpriteRenderer>(e, { glm::vec3(1.0f), tex });

        auto cubeMesh = lavander::Mesh::MakeCube(engine.getDevice());
        auto e2 = reg.createEntity();
        reg.addComponent<lavander::MeshFilter>(e2, { cubeMesh });
        reg.addComponent<lavander::MeshRenderer3D>(e2, { nullptr, glm::vec3(1.0f,0.0f,0.0f) });
        reg.addComponent<lavander::Transform>(e2, {{0,0,-2}, {0,0,0},{1,1,1} });


        engine.run();


        ////query components
        //auto& positions = registry.getAllComponentsOfType<Transform>();
        //for (const auto& [entity, components] : positions)
        //{
        //    for (const auto& position : components)
        //    {
        //        std::cout << "Entity " << entity << ": Transform("
        //            << position.position.x << ", " << position.position.y << ", " << position.position.z << ")\n";
        //    }
        //}
    } 
    catch (const std::exception &e) 
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}