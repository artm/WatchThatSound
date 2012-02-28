#include <igloo/igloo_alt.h>
using namespace igloo;

#include "Project.h"
using namespace WTS;

Describe(A_Project) {
    It(Should_have_default_constructor) {
        Project project;
    }
};
