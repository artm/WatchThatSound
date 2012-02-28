#include <igloo/igloo_alt.h>
using namespace igloo;

#include "Project.h"
using namespace WTS;

Describe ( A_Project_constructed_witout_a_file ) {
    Project _project;
    Project& project() { return _project; }

    It ( should_be_invalid ) {
        Assert::That( project().videoFile() == NULL );
        Assert::That( !project().isValid() );
    }

    Describe ( Invalid_Project ) {
        Project& project() { return Parent().project(); }

        It ( should_throw_when_accessed ) {
            AssertThrows( Project::InvalidProject, project().addMarker(Project::EVENT, 0, 0.5) );
            AssertThrows( Project::InvalidProject, project().getMarkers() );
            AssertThrows( Project::InvalidProject, project().tensionCurve(100.0f) );
            AssertThrows( Project::InvalidProject, project().duration() );
            // etc ...
        }
    };

    When(duration_is_set) {
        Project& project() { Parent().project(); }

        void SetUp() {
            project().setDuration(1000);
        }

        It ( should_become_valid ) {
            Assert::That( project().isValid() );
        }

        It ( should_return_set_duration ) {
            AssertThat(project().duration(), Equals(1000));
        }

    };
};
