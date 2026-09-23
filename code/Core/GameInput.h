//インプット関連
#pragma once	

namespace GameInput
{
	struct Input
	{
        int xDir = 0;
        int yDir = 0;

        bool selectPressed = false;
        bool backPressed = false;
        bool confirmPressed = false;
	};

    //インプット結果
    enum class InputResult
    {
        None,
        OpenMenu,
        CloseMenu
    };

    //インプット受付用インタフェース
    class IInputReceiver
    {
    public:
        virtual ~IInputReceiver() = default;

        virtual InputResult HandleInput(const Input& input) = 0;
    };

}
