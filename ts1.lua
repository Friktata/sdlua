

    print("Running script one")
    
    function tf(str) 

        print("Function tf: str = " .. str)

    end

    status("ALL GOOD")

    envs = listenv()

    for key, env in pairs(envs) do
        print(key .. " = " .. envs[key])

        scripts = listenv(envs[key])

        if (type(scripts) == "string") then
            print("Error: " .. scripts)
            os.exit(1)
        end

        for s, scr in pairs(scripts) do
            print("\tScript " .. s)
        end
    end
