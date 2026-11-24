#pragma once

#if defined(GL_VERSION_1_0) || defined(GLAD_GL_H_) || defined(VK_VERSION_1_0)
#error "Backend header leaked into RHI interface layer!"
#endif // defined(GL_VERSION_1_0) || defined(GLAD_GL_H_) ||
       // defined(VK_VERSION_1_0)
