import { Panel } from './Panel';
import { EventEmitter } from './Events';

export class EditorPanel {
    protected title: string;
    protected panels: Map<string, Panel>;
    protected commandHistory: CommandHistory;
    protected eventEmitter: EventEmitter<any>;

    constructor(title: string) {
        this.title = title;
        this.panels = new Map();
        this.commandHistory = new CommandHistory();
        this.eventEmitter = new EventEmitter();
    }

    protected addToolbarButton(label: string, callback: () => void): void {
        // Add button to toolbar with specified label and callback
    }

    protected createWorkspaceSection(title: string, panel: Panel): void {
        this.panels.set(title, panel);
    }

    protected addPropertyInspector(): PropertyInspector {
        const inspector = new PropertyInspector();
        this.panels.set('Property Inspector', inspector);
        return inspector;
    }

    protected requestSceneUpdate(): void {
        // Trigger scene update in the 3D viewport
        this.eventEmitter.emit('sceneUpdate', null);
    }
}

class CommandHistory {
    private undoStack: Command[];
    private redoStack: Command[];

    constructor() {
        this.undoStack = [];
        this.redoStack = [];
    }

    public undo(): void {
        const command = this.undoStack.pop();
        if (command) {
            command.undo();
            this.redoStack.push(command);
        }
    }

    public redo(): void {
        const command = this.redoStack.pop();
        if (command) {
            command.execute();
            this.undoStack.push(command);
        }
    }

    public execute(command: Command): void {
        command.execute();
        this.undoStack.push(command);
        this.redoStack = []; // Clear redo stack when new command is executed
    }
}

interface Command {
    execute(): void;
    undo(): void;
}

class PropertyInspector extends Panel {
    private categories: Map<string, PropertyCategory>;

    constructor() {
        super('Property Inspector');
        this.categories = new Map();
    }

    public addCategory(name: string): PropertyCategory {
        const category = new PropertyCategory(name);
        this.categories.set(name, category);
        return category;
    }
}

class PropertyCategory {
    private name: string;
    private properties: Map<string, any>;

    constructor(name: string) {
        this.name = name;
        this.properties = new Map();
    }

    public addProperty(name: string, value: any): void {
        this.properties.set(name, value);
    }
}
