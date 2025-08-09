#include "engine.hpp"
#include "components.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
    lavander::Engine engine{};

    try 
    {
        auto& reg = engine.getRegistry();
        auto e = reg.createEntity();
        auto e2 = reg.createEntity();

        reg.addComponent<Transform>(e, 
            {
                glm::vec3(0.0f,0.0f,0.0f),
                glm::vec3(0.0f, glm::radians(0.0f), 0.0f),
                glm::vec3(0.5f, 0.5f, 0.5f)          
            
            });

        reg.addComponent<SpriteRenderer>(e, { glm::vec3(1.0f, 0.0f, 0.0f) });

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