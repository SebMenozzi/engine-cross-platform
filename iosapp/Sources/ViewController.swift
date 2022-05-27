import UIKit
import MetalKit

final class ViewController: UIViewController {
    
    private let engine = EngineWrapper()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        view.backgroundColor = .red
        
        let metalView = MTKView(frame: view.frame)
        view.addSubview(metalView)
        
        metalView.frame = view.frame
        
        engine.create()
        engine.initializeViewAndAssetsPath(metalView, path: Bundle.main.resourcePath)
        engine.update(0)
    }
}
