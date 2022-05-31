import UIKit
import MetalKit
import CoreMotion

final class ViewController: UIViewController {
    
    private let engineWrapper = EngineWrapper()
    
    private let motionManager = CMMotionManager()
    private var motionTimer: Timer?
    
    private var displayLink: CADisplayLink?
    private var startTime = 0.0
    
    private func startDisplayLink() {
        stopDisplayLink()
        startTime = CACurrentMediaTime()
        
        displayLink = CADisplayLink(target: self, selector: #selector(displayLinkDidFire))
        
        displayLink?.preferredFramesPerSecond = UIScreen.main.maximumFramesPerSecond
        displayLink?.add(to: RunLoop.current, forMode: .commonModes)
    }
    
    private func stopDisplayLink() {
        displayLink?.invalidate()
        displayLink = nil
    }
    
    @objc private func displayLinkDidFire(_ displayLink: CADisplayLink) {
        let elapsedTime = CACurrentMediaTime() - startTime
        
        engineWrapper.update(elapsedTime)
    }
    
    private func startCameraCoreMotion() {
        let updateInterval = 1.0 / 60.0
        motionManager.deviceMotionUpdateInterval = updateInterval
        motionManager.showsDeviceMovementDisplay = true
        motionManager.startDeviceMotionUpdates(using: .xTrueNorthZVertical)
        
        if !motionManager.isDeviceMotionAvailable {
            print("Device motion not available")
            return
        }
        
        // Configure a timer to fetch the motion data.
        let motionTimer = Timer(
            fire: Date(),
            interval: updateInterval,
            repeats: true,
            block: { [weak self] (timer) in
            if let self = self,
               let data = self.motionManager.deviceMotion {
                let pitch = data.attitude.pitch
                let yaw = data.attitude.yaw
                let roll = data.attitude.roll
                
                self.engineWrapper.sendCameraEvent(
                    withPitchYawRoll: pitch.radiansToDegrees - 90,
                    yaw: yaw.radiansToDegrees,
                    roll: roll.radiansToDegrees
                )
            }
        })
        
        RunLoop.current.add(motionTimer, forMode: .defaultRunLoopMode)
        
        self.motionTimer = motionTimer
    }
    
    private func setupNotifications() {
        NotificationCenter.default.addObserver(self,
            selector: #selector(resume),
            name: .UIApplicationDidBecomeActive,
            object: nil
        )
        
        NotificationCenter.default.addObserver(self,
            selector: #selector(resume),
            name: .UIApplicationWillEnterForeground,
            object: nil
        )
        
        NotificationCenter.default.addObserver(self,
            selector: #selector(pause),
            name: .UIApplicationDidEnterBackground,
            object: nil
        )
        
        NotificationCenter.default.addObserver(self,
            selector: #selector(pause),
            name: .UIApplicationWillResignActive,
            object: nil
        )
    }
    
    @objc private func resume() {
        startCameraCoreMotion()
        
        displayLink?.isPaused = false
    }
    
    @objc private func pause() {
        motionTimer?.invalidate()
        motionTimer = nil
        
        displayLink?.isPaused = true
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        
        let metalView = MTKView(frame: view.frame)
        metalView.preferredFramesPerSecond = UIScreen.main.maximumFramesPerSecond
        view.addSubview(metalView)
        
        metalView.frame = view.frame
        
        engineWrapper.create()
        engineWrapper.initializeViewAndAssetsPath(metalView, path: Bundle.main.resourcePath)
        
        startDisplayLink()
        startCameraCoreMotion()
        
        setupNotifications()
    }
    
    override var supportedInterfaceOrientations: UIInterfaceOrientationMask {
        return .portrait
    }
}
