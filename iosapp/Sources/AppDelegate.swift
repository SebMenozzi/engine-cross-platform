import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {
    
    var window: UIWindow?

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Create the window (bypass the storyboard)
        window = UIWindow(frame: UIScreen.main.bounds)
        // Make this window visible
        window?.makeKeyAndVisible()
        window?.rootViewController = ViewController()

        return true
    }
}
