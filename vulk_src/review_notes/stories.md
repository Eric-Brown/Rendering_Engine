We can break down the structure of a user story as:

    The brief description of the need
    The conversations that happen during backlog grooming and sprint planning to solidify the details
    The acceptance tests that confirm the story’s satisfactory completion

As a rule of thumb, a good user story should adhere to the INVEST acronym:

Independent – user stories should not depend on each other so they can be developed in any order.

Negotiable – Avoid too much detail; keep them flexible so the team can adjust how much of the story to implement.

Valuable – the story should provide some value to its users.

Estimable – the team must be able to estimate the story.

Small – user stories should be small enough to fit in a sprint; large stories are hard to estimate and plan.

Testable – ensure what is being developed can be verified and tested adequately.


Features/User Stories

Feature stories are designed to explain the who, what, and why of the smallest incremental feature that can be added to a product that delivers user value. Feature stories are pointed by the development team and are estimated by complexity rather than by the time it will take to complete the feature. They are written from the user’s perspective and act as lightweight requirements documentation for a development team. Following the INVEST model, they should be independent and create clear value to the user. For example, each button should be added in conjunction with a feature instead of having a story that creates a row of buttons that are not connected to features. While features and user stories are often used interchangeably, for the sake of clarity—and to be consistent with Tracker’s naming conventions —we will refer to this kind of story only as a feature story throughout the rest of this post. Feature stories should include several things:

    Title: The title should be short, descriptive, and include the specific user/persona. For example, the user/persona should either be the specific type of user (e.g., editor) or the persona name (e.g., Jill) rather than just “user.” This also works for users that are not people but systems (e.g., “Purchasing API”).
    Business case: Who wants the functionality, why, and to what end? This is incorporated so that everyone on the team understands why the feature is being added. If you cannot think of a reason, then it’s worth reassessing if the feature should be included. The business case also allows others on the team to think if there is a better user experience than the one provided.
    Acceptance criteria: This defines what you will walk through to make sure that the story is completed. Developers working on the story should also walk through the acceptance criteria before delivering. At Pivotal, our beginning template for acceptance criteria is written in the Gherkin language. The Gherkin language was originally created for Cucumber, a Ruby testing suite, and is meant to “enforce firm, unambiguous requirements…and a script for automated testing.” Keep in mind that good feature stories should not, however, be prescriptive in implementation details. The syntax is as follows: GIVEN [necessary context] WHEN [action] THEN [reaction]. Sometimes it helps to walk backwards to determine your GIVEN (examples to follow). If you find yourself writing multiple “and”s in the acceptance criteria, that is a smell test that indicates you should split the story into two or more.
    Notes: Include additional information needed for the story, such as design notes (which might point out changes to mocks) or developer notes (which might provide insight that could help developers deliver the story).
    Resources: Add mocks, wireframes, user flows, links, and other assets to help deliver the feature story.
    Labels: This includes epics (larger overarching features), build #s, users, etc. These are used most effectively to help group the stories together.

User stories generally have the following format:

As a \<user type\>, I want to \<feature\> so that \<benefit\>.

Narrative

    The first part of the user story is the Narrative. 2-3 sentences used to describe the intent of the story. It is just a summary of the intent.

Conversations

    The most crucial aspect of a user story is the conversations that should happen continuously between the development team, customer, Product Owner and other stakeholders to solidify the details of the user story.

Acceptance Criteria

    Acceptance criteria represent the conditions of satisfaction which are written as scenarios, usually in Gherkin (Given, When, Then) format. Acceptance criteria also provide the Definition of Done for the story.
    
Common Mistakes When Writing User Stories

Too formal or too much detail. Product owners with good intentions often try to write extremely detailed user stories.  If a team sees a story at iteration planning that looks like an IEEE requirements document, they often assume that all the details are there and will skip the detailed conversation.

Writing user stories for Technical tasks. Much of the power of Agile comes from having a working increment of software at the end of each iteration.  If your stories are really just technical tasks, you often do not end up with working software at the end of each iteration, and you lose flexibility in prioritization.

Skipping the conversation. Stories are intentionally vague before iteration planning.  If you skip the acceptance criteria conversation, you risk moving in the wrong direction, missing edge cases or overlooking customer needs.


Bugs

A bug is a defect in a feature that has already been accepted, regardless of when it was accepted. You should not use bugs to detail new features and functionality (e.g., “Price should be non-negative” or “Login button doesn’t work”). Bugs are directly related to features that have already been delivered and provide no new user value, which is why they do not have points. The other reason they do not have points is that bugs can be impossible to estimate and could take 30 seconds or 30 days to resolve.

Bugs should include this information:

    Title: This should be short and descriptive.
    Description: What is currently happening? What should be happening?
    Instructions: Outline the steps to reproduce it.
    Resources: Include screenshots and other assets to help explain/show the bug.

Chores

A chore is a story that is necessary but provides no direct, obvious value to the user (e.g., “Setup new domain and wildcard SSL certificate to create test environments” or “Evaluate tools for system troubleshooting”). Chores can represent technical debt and/or points of dependency on other teams. Chores are not estimated (i.e., pointed), as they do not contribute user value directly. This means that if a chore feels like it provides user value then it should be incorporated into a feature story. For example, if you are using an analytics service, the additional complexity of the service setup should be taken into account in the first analytics feature story, rather than separated out as a chore.

Chores should include this information:

    Title: This should be short and descriptive.
    Description: Why is it needed? Does it help the team go faster or is it a dependency that could cause problems in the codebase if it’s not done?
    Resources: Including instructions, additional context, or other assets that help execute the chore.
