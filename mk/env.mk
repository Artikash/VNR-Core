# env.mk
# 8/8/2013 jichi

ifeq ($(OS),Windows_NT)
  COFFEE = coffee
  CLOSURE = closure.cmd
else
  COFFEE = coffee
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
  CLOSURE = closure
endif

.coffee:
	$(COFFEE) -c $(INPUT)

.closure:
	$(CLOSURE) --js_output_file $(OUTPUT) --js $(INPUT)

# EOF
