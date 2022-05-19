import Cocoa
import MetalKit

class AppDelegate: NSObject, NSApplicationDelegate {

    private var window: NSWindow!

    private let wrapper = Wrapper()

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        let rect = NSRect(x: 0, y: 0, width: 1280, height: 720)

        window = NSWindow(
            contentRect: rect,
            styleMask: [.miniaturizable, .closable, .resizable, .titled],
            backing: .buffered, defer: false
        )
        window.center()
        window.title = "Engine"
        window.makeKeyAndOrderFront(nil)

        let view = MTKView(frame: rect)
        window.contentView = view

        wrapper.createApplication()
        wrapper.initialize(view)
        wrapper.start()
    }
}