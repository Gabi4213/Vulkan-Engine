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
        auto& registry = engine.getRegistry();

        Entity e1 = registry.createEntity();
        registry.addComponent<Transform>(e1, { glm::vec3(1.0f, 2.0f, 3.0f) });

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