#import "MVKView.h"

@implementation MVKView
{
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.renderMode = Diligent::MacOSAppBase::RenderMode::MoltenVK;
    }
    return self;
}

- (id)initWithCoder:(NSCoder*)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        self.renderMode = Diligent::MacOSAppBase::RenderMode::MoltenVK;
    }
    return self;
}

@end
