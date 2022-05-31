import UIKit
import UIKit.UIGestureRecognizerSubclass

class CustomGestureRecognizer: UIPinchGestureRecognizer {
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent) {
        super.touchesBegan(touches, with: event)
        
        print("TOUCH BEGAN")
        
        for touch in touches {
            print(touch.type)
        }
    }
    
    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent) {
        super.touchesMoved(touches, with: event)
        
        print("TOUCH MOVED")
        
        for touch in touches {
            print(touch.type)
        }
    }
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent) {
        super.touchesEnded(touches, with: event)
        
        print("TOUCH ENDED")
        
        for touch in touches {
            print(touch.type)
        }
    }
}
