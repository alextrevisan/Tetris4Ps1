.section .data

.global background
.type background, @object

.global tiles
.type tiles, @object

.global balls
.type balls, @object

background:
	.incbin "textures/BACKGROUND.TIM"

tiles:
	.incbin "textures/TILES.TIM"

balls:
	.incbin "textures/TILES.TIM"

