vec4 colorAttachmentReadEXT(attachmentEXT _Attachment);
ivec4 colorAttachmentReadEXT(iattachmentEXT _Attachment);
uvec4 colorAttachmentReadEXT(uattachmentEXT _Attachment);
vec4 colorAttachmentReadEXT(attachmentEXT _Attachment, int _Sample);
ivec4 colorAttachmentReadEXT(iattachmentEXT _Attachment, int _Sample);
uvec4 colorAttachmentReadEXT(uattachmentEXT _Attachment, int _Sample);

float depthAttachmentReadEXT();
float depthAttachmentReadEXT(int _Sample);

uint stencilAttachmentReadEXT();
uint stencilAttachmentReadEXT(int _Sample);
