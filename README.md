# UEDevTools
_Just a set of tools built using imgui for Unreal Engine._


# How
- Add the [UnrealImgui](https://github.com/trdwll/UnrealImGui) plugin to your project.
- Add the UEDevTools folder to your Source directory in your project.
- In your PlayerController or really wherever Spawn the actor __ACDebugToolsActor__. - (_currently you have to use ImGui.ToggleInput as a console command to interact with Imgui_)
- Add "RHI", "RenderCore" to your Build.cs file also
- Just a note, you should probably strip these tools out of Shipping builds. atm I'm not doing that

# Why
lol why not?

I just wanted a powerful toolset that I could use in standalone.



# TODO
uh... these are just ideas...
- mini-profilier (including net)
- ability to edit data in the Inspector?
- style imgui to be like the engine
- move into a plugin for ease of implementation? then again it's already pretty easy to add...