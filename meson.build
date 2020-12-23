project('vkcompute', 'c',
  version : '0.1',
  default_options : ['c_std=c11', 'warning_level=3'])

cc = meson.get_compiler('c')

glslc = find_program('glslc')

shader_target = custom_target(
	'shaders',
	input: ['shader.comp'],
	output: ['shader.spv'],
	command: [glslc, '-c', '@INPUT@'],
	install: true,
	install_dir: '/'
)

depends = [dependency('vulkan', required: true)]

if build_machine.system() == 'darwin'
	depends += cc.find_library('MoltenVK', required: true)
endif

executable('vkcompute',
           'grabShaderModule.c',
           'findForInit.c',
           'fromFile.c',
           install : true,
           dependencies : depends)
